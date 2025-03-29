// sui_core.cpp

#ifndef SUI_CORE_CPP
#define SUI_CORE_CPP

//~ implementation

//- state 

function void
sui_init() {
    
}

function void
sui_release() {
    
}

function void 
sui_begin(sui_context_t* context) {
    sui_active_context = context;
}

function void 
sui_end(sui_context_t* context) {
    sui_active_context = nullptr;
}

function arena_t*
sui_build_arena() {
    arena_t* arena = sui_active_context->build_arenas[sui_active_context->build_index % 2];
    return arena;
}

//- context functions 

function sui_context_t* 
sui_context_create(os_handle_t window, gfx_handle_t renderer) {
    
    arena_t* arena = arena_create(megabytes(256));
    sui_context_t* context = (sui_context_t*)arena_alloc(arena, sizeof(sui_context_t));
    memset(context, 0, sizeof(sui_context_t));
    
    context->arena = arena;
    context->build_arenas[0] = arena_create(megabytes(64));
    context->build_arenas[1] = arena_create(megabytes(64));
    context->drag_state_arena = arena_create(megabytes(64));
    
    context->window = window;
    context->renderer = renderer;
    
    context->node_hash_list_count = 4096;
    context->node_hash_list = (sui_node_hash_list_t*)arena_alloc(arena, sizeof(sui_node_hash_list_t) * context->node_hash_list_count);
    
    context->anim_hash_list_count = 4096;
    context->anim_hash_list = (sui_anim_hash_list_t*)arena_alloc(arena, sizeof(sui_anim_hash_list_t) * context->anim_hash_list_count);
    
    return context;
}

function void 
sui_context_release(sui_context_t* context) {
    arena_release(context->drag_state_arena);
    arena_release(context->build_arenas[0]);
    arena_release(context->build_arenas[1]);
    arena_release(context->arena);
}

//- key functions  

function sui_key_t 
sui_key_from_string(sui_key_t seed, str_t string) {
    
    sui_key_t result = { 0 };
    
    if (string.size != 0) {
        
        // find hash portion of the string string
        str_t hash_string = string;
        u32 pos = str_find_substr(string, str("###"));
        if (pos < string.size) {
            hash_string = str_skip(string, pos);
        }
        result.data[0] = str_hash(seed.data[0], hash_string);
        
    }
    
    return result;
}

function sui_key_t
sui_key_from_stringf(sui_key_t seed, char* fmt, ...) {
    temp_t scratch = scratch_begin();
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(scratch.arena, fmt, args);
    va_end(args);
    
    sui_key_t result = sui_key_from_string(seed, string);
    
    scratch_end(scratch);
    return result;
}

function b8
sui_key_equals(sui_key_t a, sui_key_t b) {
    b8 result = (a.data[0] == b.data[0]);
    return result;
}

//- size functions 

inlnfunc sui_size_t 
sui_size(sui_size_type type, f32 value, f32 strictness) {
    return { type, value, strictness };
}

inlnfunc sui_size_t 
sui_size_pixels(f32 pixels, f32 strictness) {
    return { sui_size_type_pixel, pixels, strictness };
}

inlnfunc sui_size_t 
sui_size_percent(f32 percent) {
    return { sui_size_type_percent, percent, 1.0f };
}

inlnfunc sui_size_t 
sui_size_by_children(f32 strictness) {
    return { sui_size_type_by_children, 0.0f, strictness };
}

inlnfunc sui_size_t 
sui_size_by_text(f32 padding) {
    return { sui_size_type_by_text, padding, 1.0f };
}

//- axis/side/dir functions  

function sui_axis 
sui_axis_from_dir(sui_dir dir) {
    // sui_dir_left -> sui_axis_x;
    // sui_dir_up -> sui_axis_y;
    // sui_dir_right -> sui_axis_x;
    // sui_dir_down -> sui_axis_y;
    return ((dir & 1) ? sui_axis_y : sui_axis_x);
}

function sui_side 
sui_side_from_dir(sui_dir dir) {
    // sui_dir_left -> sui_side_min;
    // sui_dir_up -> sui_side_min;
    // sui_dir_right -> sui_side_max;
    // sui_dir_down -> sui_side_max;
    return ((dir < 2) ? sui_side_min : sui_side_max);
}

function sui_dir
sui_dir_from_axis_side(sui_axis axis, sui_side side) {
    sui_dir result = (axis == sui_axis_x) ? 
    (side == sui_side_min) ? sui_dir_left : sui_dir_right :
    (side == sui_side_min) ? sui_dir_up : sui_dir_down;
    return result;
}

//- node functions 

function sui_node_t* 
sui_node_find(sui_key_t key) {
    
    sui_node_t* result = nullptr;
    
    if (!sui_key_equals(key, { 0 })) {
        u32 index = key.data[0] % sui_active_context->node_hash_list_count;
        for (sui_node_t* node = sui_active_context->node_hash_list[index].first; node != nullptr; node = node->list_next) {
            if (sui_key_equals(node->key, key)) {
                result = node;
                break;
            }
        }
    }
    
    return result;
}

function sui_node_t* 
sui_node_from_key(sui_node_flags flags, sui_key_t key) {
    
    sui_context_t* context = sui_active_context;
    sui_node_t* node = nullptr;
    
    // try to find existing node.
    node = sui_node_find(key);
    
    // filter out duplicates
    b8 node_is_new = (node == nullptr);
    if (!node_is_new && (node->last_build_index == context->build_index)) {
        node = nullptr;
        key = { 0 };
        node_is_new = true;
    }
    
    b8 node_is_transient = sui_key_equals(key, { 0 });
    
    // grab from free list or allocate one
    if (node_is_new) {
        node = node_is_transient ? nullptr : context->node_free;
        if (node != nullptr) {
            stack_pop_n(context->node_free, list_next);
        } else {
            node = (sui_node_t*)arena_alloc(node_is_transient ? sui_build_arena() : context->arena, sizeof(sui_node_t));
        }
        memset(node, 0, sizeof(sui_node_t));
        node->first_build_index = context->build_index;
    }
    
    // clear node
    node->tree_next = nullptr;
    node->tree_prev = nullptr;
    node->tree_parent = nullptr;
    node->tree_first = nullptr;
    node->tree_last = nullptr;
    node->flags = 0;
    
    // add to node list if needed
    if (node_is_new && !node_is_transient) {
        u32 index = key.data[0] % context->node_hash_list_count;
        dll_push_back_np(context->node_hash_list[index].first, context->node_hash_list[index].last, node, list_next, list_prev);
    }
    
    // add to node tree if needed
    sui_node_t* parent = sui_top_parent();
    if (parent != nullptr) {
        dll_push_back_np(parent->tree_first, parent->tree_last, node, tree_next, tree_prev);
        node->tree_parent = parent;
    }
    
    // fill node members
    node->key = key;
    node->group_key = sui_top_group_key();
    node->flags = (flags | sui_top_flags()) & ~sui_top_omit_flags();
    node->pos_target.x = sui_top_fixed_pos_x();
    node->pos_target.y = sui_top_fixed_pos_y();
    node->size_target.x = sui_top_fixed_size_x();
    node->size_target.y = sui_top_fixed_size_y();
    node->size_wanted[0] = sui_top_size_x();
    node->size_wanted[1] = sui_top_size_y();
    node->padding.x = sui_top_padding_x();
    node->padding.y = sui_top_padding_y();
    node->layout_dir = sui_top_layout_dir();
    node->text_alignment = sui_top_text_alignment();
    node->rounding.x = sui_top_rounding_00();
    node->rounding.y = sui_top_rounding_01();
    node->rounding.z = sui_top_rounding_10();
    node->rounding.w = sui_top_rounding_11();
    node->border_size = sui_top_border_size();
    node->shadow_size = sui_top_shadow_size();
    node->texture = sui_top_texture();
    node->font = sui_top_font();
    node->font_size = sui_top_font_size();
    
    // tags
    node->tags_key = { 0 };
    if (sui_active_context->tags_stack_top != nullptr) {
        node->tags_key = sui_active_context->tags_stack_top->key;
    }
    
    // auto pop stacks
    sui_auto_pop_stacks();
    
}

function sui_node_t*
sui_node_from_string(sui_node_flags flags, str_t string) {
    
    sui_key_t seed_key = sui_top_seed_key();
    sui_key_t key = sui_key_from_string(seed_key, string);
    sui_node_t* node = sui_node_from_key(flags, key);
    node->label = string;
    return node;
}

function sui_node_t*
sui_node_from_stringf(sui_node_flags flags, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(sui_build_arena(), fmt, args);
    va_end(args);
    
    sui_node_t* node = sui_node_from_string(flags, string);
    return node;
}

//- stack functions 

function void 
sui_auto_pop_stacks() {
#define sui_stack(name, type) if (sui_active_context->name##_stack.auto_pop) { sui_pop_##name(); sui_active_context->name##_stack.auto_pop = false; };
    sui_stack_list
#undef sui_stack
}

#define sui_stack_top_impl(name, type)\
return sui_active_context->name##_stack.top->v;

#define sui_stack_push_impl(name, type)\
sui_##name##_node_t* node = sui_active_context->name##_stack.free;\
if (node != nullptr) {\
stack_pop(sui_active_context->name##_stack.free);\
} else {\
node = (sui_##name##_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_##name##_node_t));\
}\
type old_value = sui_active_context->name##_stack.top->v; node->v = v;\
stack_push(sui_active_context->name##_stack.top, node);\
sui_active_context->name##_stack.auto_pop = false;\
return old_value;


#define sui_stack_pop_impl(name, type, default)\
sui_##name##_node_t* popped = sui_active_context->name##_stack.top;\
type result = default;\
if (popped != nullptr) {\
result = popped->v;\
stack_pop(sui_active_context->name##_stack.top);\
stack_push(sui_active_context->name##_stack.free, popped);\
sui_active_context->name##_stack.auto_pop = false;\
}\
return result;


#define sui_stack_set_next_impl(name, type)\
sui_##name##_node_t* node = sui_active_context->name##_stack.free;\
if (node != nullptr) {\
stack_pop(sui_active_context->name##_stack.free);\
} else {\
node = (sui_##name##_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_##name##_node_t));\
}\
type old_value = sui_active_context->name##_stack.top->v;\
node->v = v;\
stack_push(sui_active_context->name##_stack.top, node);\
sui_active_context->name##_stack.auto_pop = true;\
return old_value;

function sui_node_t* sui_top_parent() { sui_stack_top_impl(parent, sui_node_t*) }
function sui_node_t* sui_push_parent(sui_node_t* v) { sui_stack_push_impl(parent, sui_node_t*) }
function sui_node_t* sui_pop_parent() { sui_stack_pop_impl(parent, sui_node_t*, nullptr) }
function sui_node_t* sui_set_next_parent(sui_node_t* v) { sui_stack_set_next_impl(parent, sui_node_t*) }

function sui_node_flags sui_top_flags() { sui_stack_top_impl(flags, sui_node_flags) }
function sui_node_flags sui_push_flags(sui_node_flags v) { sui_stack_push_impl(flags, sui_node_flags) }
function sui_node_flags sui_pop_flags() { sui_stack_pop_impl(flags, sui_node_flags, 0) }
function sui_node_flags sui_set_next_flags(sui_node_flags v) { sui_stack_set_next_impl(flags, sui_node_flags) }

function sui_node_flags sui_top_omit_flags() { sui_stack_top_impl(omit_flags, sui_node_flags) }
function sui_node_flags sui_push_omit_flags(sui_node_flags v) { sui_stack_push_impl(omit_flags, sui_node_flags) }
function sui_node_flags sui_pop_omit_flags() { sui_stack_pop_impl(omit_flags, sui_node_flags, 0) }
function sui_node_flags sui_set_next_omit_flags(sui_node_flags v) { sui_stack_set_next_impl(omit_flags, sui_node_flags) }

function sui_key_t sui_top_seed_key() { sui_stack_top_impl(seed_key, sui_key_t) }
function sui_key_t sui_push_seed_key(sui_key_t v) { sui_stack_push_impl(seed_key, sui_key_t) }
function sui_key_t sui_pop_seed_key() { sui_stack_pop_impl(seed_key, sui_key_t, { 0 }) }
function sui_key_t sui_set_next_seed_key(sui_key_t v) { sui_stack_set_next_impl(seed_key, sui_key_t) }

function sui_key_t sui_top_group_key() { sui_stack_top_impl(group_key, sui_key_t) }
function sui_key_t sui_push_group_key(sui_key_t v) { sui_stack_push_impl(group_key, sui_key_t) }
function sui_key_t sui_pop_group_key() { sui_stack_pop_impl(group_key, sui_key_t, { 0 }) }
function sui_key_t sui_set_next_group_key(sui_key_t v) { sui_stack_set_next_impl(group_key, sui_key_t) }

function f32 sui_top_fixed_size_x() { sui_stack_top_impl(fixed_size_x, f32) }
function f32 sui_push_fixed_size_x(f32 v) { sui_stack_push_impl(fixed_size_x, f32) }
function f32 sui_pop_fixed_size_x() { sui_stack_pop_impl(fixed_size_x, f32, 0.0f) }
function f32 sui_set_next_fixed_size_x(f32 v) { sui_stack_set_next_impl(fixed_size_x, f32) }

function f32 sui_top_fixed_size_y() { sui_stack_top_impl(fixed_size_y, f32) }
function f32 sui_push_fixed_size_y(f32 v) { sui_stack_push_impl(fixed_size_y, f32) }
function f32 sui_pop_fixed_size_y() { sui_stack_pop_impl(fixed_size_y, f32, 0.0f) }
function f32 sui_set_next_fixed_size_y(f32 v) { sui_stack_set_next_impl(fixed_size_y, f32) }

function f32 sui_top_fixed_pos_x() { sui_stack_top_impl(fixed_pos_x, f32) }
function f32 sui_push_fixed_pos_x(f32 v) { sui_stack_push_impl(fixed_pos_x, f32) }
function f32 sui_pop_fixed_pos_x() { sui_stack_pop_impl(fixed_pos_x, f32, 0.0f) }
function f32 sui_set_next_fixed_pos_x(f32 v) { sui_stack_set_next_impl(fixed_pos_x, f32) }

function f32 sui_top_fixed_pos_y() { sui_stack_top_impl(fixed_pos_y, f32) }
function f32 sui_push_fixed_pos_y(f32 v) { sui_stack_push_impl(fixed_pos_y, f32) }
function f32 sui_pop_fixed_pos_y() { sui_stack_pop_impl(fixed_pos_y, f32, 0.0f) }
function f32 sui_set_next_fixed_pos_y(f32 v) { sui_stack_set_next_impl(fixed_pos_y, f32) }

function sui_size_t sui_top_size_x() { sui_stack_top_impl(size_x, sui_size_t) }
function sui_size_t sui_push_size_x(sui_size_t v) { sui_stack_push_impl(size_x, sui_size_t) }
function sui_size_t sui_pop_size_x() { sui_stack_pop_impl(size_x, sui_size_t, { 0 }) }
function sui_size_t sui_set_next_size_x(sui_size_t v) { sui_stack_set_next_impl(size_x, sui_size_t) }

function sui_size_t sui_top_size_y() { sui_stack_top_impl(size_y, sui_size_t) }
function sui_size_t sui_push_size_y(sui_size_t v) { sui_stack_push_impl(size_y, sui_size_t) }
function sui_size_t sui_pop_size_y() { sui_stack_pop_impl(size_y, sui_size_t, { 0 }) }
function sui_size_t sui_set_next_size_y(sui_size_t v) { sui_stack_set_next_impl(size_y, sui_size_t) }

function f32 sui_top_padding_x() { sui_stack_top_impl(padding_x, f32) }
function f32 sui_push_padding_x(f32 v) { sui_stack_push_impl(padding_x, f32) }
function f32 sui_pop_padding_x() { sui_stack_pop_impl(padding_x, f32, 0.0f) }
function f32 sui_set_next_padding_x(f32 v) { sui_stack_set_next_impl(padding_x, f32) }

function f32 sui_top_padding_y() { sui_stack_top_impl(padding_y, f32) }
function f32 sui_push_padding_y(f32 v) { sui_stack_push_impl(padding_y, f32) }
function f32 sui_pop_padding_y() { sui_stack_pop_impl(padding_y, f32, 0.0f) }
function f32 sui_set_next_padding_y(f32 v) { sui_stack_set_next_impl(padding_y, f32) }

function sui_dir sui_top_layout_dir() { sui_stack_top_impl(layout_dir, sui_dir) }
function sui_dir sui_push_layout_dir(sui_dir v) { sui_stack_push_impl(layout_dir, sui_dir) }
function sui_dir sui_pop_layout_dir() { sui_stack_pop_impl(layout_dir, sui_dir, 0) }
function sui_dir sui_set_next_layout_dir(sui_dir v) { sui_stack_set_next_impl(layout_dir, sui_dir) }

function sui_text_alignment sui_top_text_alignment() { sui_stack_top_impl(text_alignment, sui_text_alignment) }
function sui_text_alignment sui_push_text_alignment(sui_text_alignment v) { sui_stack_push_impl(text_alignment, sui_text_alignment) }
function sui_text_alignment sui_pop_text_alignment() { sui_stack_pop_impl(text_alignment, sui_text_alignment, 0) }
function sui_text_alignment sui_set_next_text_alignment(sui_text_alignment v) { sui_stack_set_next_impl(text_alignment, sui_text_alignment) }

function f32 sui_top_rounding_00() { sui_stack_top_impl(rounding_00, f32) }
function f32 sui_push_rounding_00(f32 v) { sui_stack_push_impl(rounding_00, f32) }
function f32 sui_pop_rounding_00() { sui_stack_pop_impl(rounding_00, f32, 0.0f) }
function f32 sui_set_next_rounding_00(f32 v) { sui_stack_set_next_impl(rounding_00, f32) }

function f32 sui_top_rounding_01() { sui_stack_top_impl(rounding_01, f32) }
function f32 sui_push_rounding_01(f32 v) { sui_stack_push_impl(rounding_01, f32) }
function f32 sui_pop_rounding_01() { sui_stack_pop_impl(rounding_01, f32, 0.0f) }
function f32 sui_set_next_rounding_01(f32 v) { sui_stack_set_next_impl(rounding_01, f32) }

function f32 sui_top_rounding_10() { sui_stack_top_impl(rounding_10, f32) }
function f32 sui_push_rounding_10(f32 v) { sui_stack_push_impl(rounding_10, f32) }
function f32 sui_pop_rounding_10() { sui_stack_pop_impl(rounding_10, f32, 0.0f) }
function f32 sui_set_next_rounding_10(f32 v) { sui_stack_set_next_impl(rounding_10, f32) }

function f32 sui_top_rounding_11() { sui_stack_top_impl(rounding_11, f32) }
function f32 sui_push_rounding_11(f32 v) { sui_stack_push_impl(rounding_11, f32) }
function f32 sui_pop_rounding_11() { sui_stack_pop_impl(rounding_11, f32, 0.0f) }
function f32 sui_set_next_rounding_11(f32 v) { sui_stack_set_next_impl(rounding_11, f32) }

function f32 sui_top_border_size() { sui_stack_top_impl(border_size, f32) }
function f32 sui_push_border_size(f32 v) { sui_stack_push_impl(border_size, f32) }
function f32 sui_pop_border_size() { sui_stack_pop_impl(border_size, f32, 0.0f) }
function f32 sui_set_next_border_size(f32 v) { sui_stack_set_next_impl(border_size, f32) }

function f32 sui_top_shadow_size() { sui_stack_top_impl(shadow_size, f32) }
function f32 sui_push_shadow_size(f32 v) { sui_stack_push_impl(shadow_size, f32) }
function f32 sui_pop_shadow_size() { sui_stack_pop_impl(shadow_size, f32, 0.0f) }
function f32 sui_set_next_shadow_size(f32 v) { sui_stack_set_next_impl(shadow_size, f32) }

function gfx_handle_t sui_top_texture() { sui_stack_top_impl(texture, gfx_handle_t) }
function gfx_handle_t sui_push_texture(gfx_handle_t v) { sui_stack_push_impl(texture, gfx_handle_t) }
function gfx_handle_t sui_pop_texture() { sui_stack_pop_impl(texture, gfx_handle_t, { 0 }) }
function gfx_handle_t sui_set_next_texture(gfx_handle_t v) { sui_stack_set_next_impl(texture, gfx_handle_t) }

function font_handle_t sui_top_font() { sui_stack_top_impl(font, font_handle_t) }
function font_handle_t sui_push_font(font_handle_t v) { sui_stack_push_impl(font, font_handle_t) }
function font_handle_t sui_pop_font() { sui_stack_pop_impl(font, font_handle_t, { 0 }) }
function font_handle_t sui_set_next_font(font_handle_t v) { sui_stack_set_next_impl(font, font_handle_t) }

function f32 sui_top_font_size() { sui_stack_top_impl(font_size, f32) }
function f32 sui_push_font_size(f32 v) { sui_stack_push_impl(font_size, f32) }
function f32 sui_pop_font_size() { sui_stack_pop_impl(font_size, f32, 0.0f) }
function f32 sui_set_next_font_size(f32 v) { sui_stack_set_next_impl(font_size, f32) }

function str_t sui_top_tag() { sui_stack_top_impl(tag, str_t) }

// tags

function str_t 
sui_push_tag(str_t v) {
    
    // get seed key
    sui_key_t seed_key = { 0 };
    if (sui_active_context->tags_stack_top != nullptr) {
        seed_key = sui_active_context->tags_stack_top->key;
    }
    
    // create key
    sui_key_t key = seed_key;
    if (v.size > 0) {
        key = sui_key_from_string(seed_key, v);
    }
    
    // get or allocate stack node
    sui_tags_stack_node_t* tags_stack_node = sui_active_context->tags_stack_free;
    if (tags_stack_node != nullptr) {
        stack_pop(sui_active_context->tags_stack_free);
    } else {
        tags_stack_node = (sui_tags_stack_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_tags_stack_node_t)); 
    }
    stack_push(sui_active_context->tags_stack_top, tags_stack_node);
    tags_stack_node->key = key;
    
    // store tag in cache
    u32 index = key.data[0] % sui_active_context->tags_hash_list_count;
    sui_tags_hash_list_t* hash_list = &sui_active_context->tags_hash_list[index];
    
    // try to find tag in cache first
    sui_tags_node_t* tag_node = nullptr;
    for (sui_tags_node_t* n = hash_list->first; n != nullptr; n = n->next) {
        if (sui_key_equals(n->key, key)) {
            tag_node = n;
            break;
        }
    }
    
    // create if we didn't find one
    if (tag_node == nullptr) {
        tag_node = (sui_tags_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_tags_node_t));
        stack_push(hash_list->first, tag_node);
        tag_node->key = key;
    }
    
    sui_stack_push_impl(tag, str_t)
}

function str_t 
sui_pop_tag() {
    if(sui_active_context->tags_stack_top != nullptr) {
        sui_tags_stack_node_t* popped = sui_active_context->tags_stack_top;
        stack_pop(sui_active_context->tags_stack_top);
        stack_push(sui_active_context->tags_stack_free, popped);
    }
    sui_stack_pop_impl(tag, str_t, { 0 })
}

function str_t 
sui_set_next_tag(str_t v) {
    
    // get seed key
    sui_key_t seed_key = { 0 };
    if (sui_active_context->tags_stack_top != nullptr) {
        seed_key = sui_active_context->tags_stack_top->key;
    }
    
    // create key
    sui_key_t key = seed_key;
    if (v.size > 0) {
        key = sui_key_from_string(seed_key, v);
    }
    
    // get or allocate stack node
    sui_tags_stack_node_t* tags_stack_node = sui_active_context->tags_stack_free;
    if (tags_stack_node != nullptr) {
        stack_pop(sui_active_context->tags_stack_free);
    } else {
        tags_stack_node = (sui_tags_stack_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_tags_stack_node_t)); 
    }
    stack_push(sui_active_context->tags_stack_top, tags_stack_node);
    tags_stack_node->key = key;
    
    // store tag in cache
    u32 index = key.data[0] % sui_active_context->tags_hash_list_count;
    sui_tags_hash_list_t* hash_list = &sui_active_context->tags_hash_list[index];
    
    // try to find tag in cache first
    sui_tags_node_t* tag_node = nullptr;
    for (sui_tags_node_t* n = hash_list->first; n != nullptr; n = n->next) {
        if (sui_key_equals(n->key, key)) {
            tag_node = n;
            break;
        }
    }
    
    // create if we didn't find one
    if (tag_node == nullptr) {
        tag_node = (sui_tags_node_t*)arena_alloc(sui_build_arena(), sizeof(sui_tags_node_t));
        stack_push(hash_list->first, tag_node);
        tag_node->key = key;
    }
    
    sui_stack_set_next_impl(tag, str_t)
}

// group stacks

function void 
sui_push_size(sui_size_t size_x, sui_size_t size_y) {
    sui_push_size_x(size_x);
    sui_push_size_y(size_y);
}

function void 
ui_pop_size() {
    sui_pop_size_x();
    sui_pop_size_y();
}

function void 
sui_set_next_size(sui_size_t size_x, sui_size_t size_y) {
    sui_set_next_size_x(size_x);
    sui_set_next_size_y(size_y);
}

function void 
sui_push_fixed_size(f32 x, f32 y) {
    sui_push_fixed_size_x(x);
    sui_push_fixed_size_y(y);
}

function void 
sui_pop_fixed_size() {
    sui_pop_fixed_size_x();
    sui_pop_fixed_size_y();
}

function void 
sui_set_next_fixed_size(f32 x, f32 y) {
    sui_set_next_fixed_size_x(x);
    sui_set_next_fixed_size_y(y);
}

function void 
sui_push_fixed_pos(f32 x, f32 y) {
    sui_push_fixed_pos_x(x);
    sui_push_fixed_pos_y(y);
}

function void 
sui_pop_fixed_pos() {
    sui_pop_fixed_pos_x();
    sui_pop_fixed_pos_y();
}

function void 
sui_set_next_fixed_pos(f32 x, f32 y) {
    sui_set_next_fixed_pos_x(x);
    sui_set_next_fixed_pos_y(y);
}


function void 
sui_push_rect(rect_t rect) {
    sui_push_fixed_pos_x(rect.x0);
    sui_push_fixed_pos_y(rect.y0);
    sui_push_fixed_size_x(rect.x1 - rect.x0);
    sui_push_fixed_size_y(rect.y1 - rect.y0);
}

function void
sui_pop_rect() {
    sui_pop_fixed_pos_x();
    sui_pop_fixed_pos_y();
    sui_pop_fixed_size_x();
    sui_pop_fixed_size_y();
}

function void 
sui_set_next_rect(rect_t rect) {
    sui_set_next_fixed_pos_x(rect.x0);
    sui_set_next_fixed_pos_y(rect.y0);
    sui_set_next_fixed_size_x(rect.x1 - rect.x0);
    sui_set_next_fixed_size_y(rect.y1 - rect.y0);
}


function void 
sui_push_rounding(vec4_t rounding) {
    sui_push_rounding_00(rounding.x);
    sui_push_rounding_01(rounding.y);
    sui_push_rounding_10(rounding.z);
    sui_push_rounding_11(rounding.w);
}

function void 
sui_pop_rounding() {
    sui_pop_rounding_00();
    sui_pop_rounding_01();
    sui_pop_rounding_10();
    sui_pop_rounding_11();
}

function void 
sui_set_next_rounding(vec4_t rounding) {
    sui_set_next_rounding_00(rounding.x);
    sui_set_next_rounding_01(rounding.y);
    sui_set_next_rounding_10(rounding.z);
    sui_set_next_rounding_11(rounding.w);
}



#endif // SUI_CORE_CPP