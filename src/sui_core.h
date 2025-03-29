// sui_core.h

#ifndef SUI_CORE_H
#define SUI_CORE_H

//- defines 

#define sui_stack_list \
sui_stack(parent, sui_node_t*)\
sui_stack(flags, sui_node_flags)\
sui_stack(omit_flags, sui_node_flags)\
sui_stack(seed_key, sui_key_t)\
sui_stack(group_key, sui_key_t)\
sui_stack(tag, str_t)\
sui_stack(fixed_size_x, f32)\
sui_stack(fixed_size_y, f32)\
sui_stack(fixed_pos_x, f32)\
sui_stack(fixed_pos_y, f32)\
sui_stack(size_x, sui_size_t)\
sui_stack(size_y, sui_size_t)\
sui_stack(padding_x, f32)\
sui_stack(padding_y, f32)\
sui_stack(layout_dir, sui_dir)\
sui_stack(text_alignment, sui_text_alignment)\
sui_stack(rounding_00, f32)\
sui_stack(rounding_01, f32)\
sui_stack(rounding_10, f32)\
sui_stack(rounding_11, f32)\
sui_stack(border_size, f32)\
sui_stack(shadow_size, f32)\
sui_stack(texture, gfx_handle_t)\
sui_stack(font, font_handle_t)\
sui_stack(font_size, f32)\

//- enums 

enum sui_size_type {
    sui_size_type_null,
    sui_size_type_pixel,
    sui_size_type_percent,
    sui_size_type_by_children,
    sui_size_type_by_text,
};

typedef u32 sui_axis;
enum {
    sui_axis_x = 0,
    sui_axis_y = 1,
    sui_axis_count,
};

typedef u32 sui_side;
enum {
    sui_side_min = 0,
    sui_side_max = 1,
    sui_side_count,
};

typedef u32 sui_dir;
enum {
    sui_dir_null = -1,
    sui_dir_left = 0,
    sui_dir_up = 1,
    sui_dir_right = 2,
    sui_dir_down = 3,
    sui_dir_count = 4,
};

typedef u32 sui_text_alignment;
enum {
    sui_text_alignment_left,
    sui_text_alignment_center,
    sui_text_alignment_right,
};

typedef u64 sui_node_flags;
enum {
    sui_flag_none = (0),
    
    // interactions
    sui_flag_mouse_interactable = (1 << 1),
    sui_flag_keyboard_interactable = (1 << 2),
    sui_flag_draggable = (1 << 3),
    sui_flag_scrollable = (1 << 4),
    sui_flag_view_scroll_x = (1 << 5),
    sui_flag_view_scroll_y = (1 << 6),
    sui_flag_view_clamp_x = (1 << 7),
    sui_flag_view_clamp_y = (1 << 8),
    
    // layout
    sui_flag_fixed_size_x = (1 << 9),
    sui_flag_fixed_size_y = (1 << 10),
    sui_flag_fixed_pos_x = (1 << 11),
    sui_flag_fixed_pos_y = (1 << 12),
    sui_flag_overflow_x = (1 << 13),
    sui_flag_overflow_y = (1 << 14),
    sui_flag_ignore_view_scroll_x = (1 << 14),
    sui_flag_ignore_view_scroll_y = (1 << 15),
    
    // appearance
    sui_flag_draw_background = (1 << 16),
    sui_flag_draw_text = (1 << 17),
    sui_flag_draw_border = (1 << 18),
    sui_flag_draw_shadow = (1 << 19),
    sui_flag_draw_hover_effects = (1 << 20),
    sui_flag_draw_active_effects = (1 << 21),
    sui_flag_draw_custom = (1 << 22),
    sui_flag_anim_size_x = (1 << 23),
    sui_flag_anim_size_y = (1 << 24),
    sui_flag_anim_pos_x = (1 << 25),
    sui_flag_anim_pos_y = (1 << 26),
    sui_flag_clip = (1 << 27),
    
    // groups
    sui_flag_interactable = sui_flag_mouse_interactable | sui_flag_keyboard_interactable,
    sui_flag_view_scroll = sui_flag_view_scroll_x | sui_flag_view_scroll_y,
    sui_flag_view_clamp = sui_flag_view_clamp_x | sui_flag_view_clamp_y,
    sui_flag_fixed_size = sui_flag_fixed_size_x | sui_flag_fixed_size_y,
    sui_flag_fixed_pos = sui_flag_fixed_pos_x | sui_flag_fixed_pos_y,
    sui_flag_overflow = sui_flag_overflow_x | sui_flag_overflow_y, 
    sui_flag_ignore_view_scroll = sui_flag_ignore_view_scroll_x | sui_flag_ignore_view_scroll_y,
    
};

typedef u32 sui_interaction;
enum {
    sui_none = (0),
    
    sui_hovered = (1 << 1),
    sui_mouse_over = (1 << 2),
    
    sui_left_pressed = (1 << 3),
    sui_middle_pressed = (1 << 4),
    sui_right_pressed = (1 << 5),
    
    sui_left_released = (1 << 6),
    sui_middle_released = (1 << 7),
    sui_right_released = (1 << 8),
    
    sui_left_clicked = (1 << 9),
    sui_middle_clicked = (1 << 10),
    sui_right_clicked = (1 << 11),
    
    sui_left_double_clicked = (1 << 12),
    sui_middle_double_clicked = (1 << 13),
    sui_right_double_clicked = (1 << 14),
    
    sui_left_triple_clicked = (1 << 15),
    sui_middle_triple_clicked = (1 << 16),
    sui_right_triple_clicked = (1 << 17),
    
    sui_left_dragging = (1 << 18),
    sui_middle_dragging = (1 << 19),
    sui_right_dragging = (1 << 20),
    
    sui_left_double_dragging = (1 << 21),
    sui_middle_double_dragging = (1 << 22),
    sui_right_double_dragging = (1 << 23),
    
    sui_left_triple_dragging = (1 << 24),
    sui_middle_triple_dragging = (1 << 25),
    sui_right_triple_dragging = (1 << 26),
    
    // TODO: unsure about keyboard interactions.
    sui_keyboard_pressed = (1 << 27),
    sui_keyboard_released = (1 << 28),
    sui_keyboard_clicked = (1 << 29),
    
};

//- typedefs 

struct sui_node_t;
typedef void sui_node_custom_draw_func(sui_node_t*);

//- structs 

// key
struct sui_key_t {
    u64 data[1];
};

// size
struct sui_size_t {
    sui_size_type type;
    f32 value;
    f32 strictness;
};

// node 
struct sui_node_t {
    
    // list
    sui_node_t* list_next;
    sui_node_t* list_prev;
    
    // tree
    sui_node_t* tree_next;
    sui_node_t* tree_prev;
    sui_node_t* tree_parent;
    sui_node_t* tree_first;
    sui_node_t* tree_last;
    
    // info
    sui_key_t key;
    sui_key_t group_key;
    sui_key_t tags_key;
    str_t label;
    sui_node_flags flags;
    
    // layout
    b8 is_transient;
    vec2_t pos_target;
    vec2_t pos;
    sui_size_t size_wanted[2];
    vec2_t size_target;
    vec2_t size;
    vec2_t padding;
    sui_dir layout_dir;
    sui_text_alignment text_alignment;
    vec2_t view_bounds;
    vec2_t view_offset_target;
    vec2_t view_offset;
    rect_t rect;
    
    // appearance
    f32 hover_t;
    f32 active_t;
    vec4_t rounding;
    f32 border_size;
    f32 shadow_size;
    gfx_handle_t texture;
    font_handle_t font;
    f32 font_size;
    os_cursor hover_cursor;
    sui_node_custom_draw_func* custom_draw_func;
    void* custom_draw_data;
    
    u64 first_build_index;
    u64 last_build_index;
};

struct sui_node_rec_t {
    sui_node_t* next;
    i32 push_count;
    i32 pop_count;
};

struct sui_node_hash_list_t {
    sui_node_t* first;
    sui_node_t* last;
    u32 count;
};

struct sui_node_list_item_t {
    sui_node_list_item_t* next;
    sui_node_t* node;
};

struct sui_node_list_t {
    sui_node_list_item_t* first;
    sui_node_list_item_t* last;
    u32 count;
};

// animation 

struct sui_anim_params_t {
    f32 initial;
    f32 target;
    f32 rate;
};

struct sui_anim_node_t {
    sui_anim_node_t* list_next;
    sui_anim_node_t* list_prev;
    
    sui_anim_node_t* lru_next;
    sui_anim_node_t* lru_prev;
    
    u64 first_build_index;
    u64 last_build_index;
    
    sui_key_t key;
    sui_anim_params_t params;
    
    f32 current;
};

struct sui_anim_hash_list_t {
    sui_anim_node_t* first;
    sui_anim_node_t* last;
    u32 count;
};

// tags

struct sui_tags_node_t {
    sui_tags_node_t* next;
    sui_key_t key;
};

struct sui_tags_hash_list_t {
    sui_tags_node_t* first;
    sui_tags_node_t* last;
};

struct sui_tags_stack_node_t {
    sui_tags_stack_node_t* next;
    sui_key_t key;
};

// stacks

#define sui_stack(name, type) \
struct sui_##name##_node_t { sui_##name##_node_t* next; type v; };\
struct sui_##name##_stack_t { sui_##name##_node_t* top; sui_##name##_node_t* free; b8 auto_pop; };
sui_stack_list
#undef sui_stack

// context

struct sui_context_t {
    
    // arenas
    arena_t* arena;
    arena_t* build_arenas[2];
    arena_t* drag_state_arena;
    
    // context
    os_handle_t window;
    gfx_handle_t renderer;
    
    // build state
    u64 build_index;
    
    // keys
    sui_key_t key_hovered;
    sui_key_t key_active[os_mouse_button_count];
    sui_key_t key_focused;
    sui_key_t key_popup;
    
    // nodes
    sui_node_hash_list_t* node_hash_list;
    u32 node_hash_list_count;
    sui_node_t* node_free;
    sui_node_t* node_root;
    sui_node_t* node_tooltip_root;
    sui_node_t* node_popup_root;
    
    // animation cache
    sui_anim_hash_list_t* anim_hash_list;
    u32 anim_hash_list_count;
    sui_anim_node_t* anim_node_free;
    sui_anim_node_t* anim_node_lru;
    sui_anim_node_t* anim_node_mru;
    
    // tags
    sui_tags_hash_list_t* tags_hash_list;
    u32 tags_hash_list_count;
    sui_tags_stack_node_t* tags_stack_top;
    sui_tags_stack_node_t* tags_stack_free;
    
    // stacks
#define sui_stack(name, type)\
sui_##name##_node_t name##default_node;\
sui_##name##_stack_t name##_stack;
    sui_stack_list
#undef sui_stack
    
};

//- globals 

thread_global sui_context_t* sui_active_context = nullptr; 

//- functions 

// state
function void sui_init();
function void sui_release();
function void sui_begin(sui_context_t* context);
function void sui_end(sui_context_t* context);
function arena_t* sui_build_arena();

// context
function sui_context_t* sui_context_create(os_handle_t window, gfx_handle_t renderer);
function void sui_context_release(sui_context_t* context);

// keys
function sui_key_t sui_key_from_string(sui_key_t seed, str_t string);
function sui_key_t sui_key_from_stringf(sui_key_t seed, char* fmt, ...);
function b8 sui_key_equals(sui_key_t a, sui_key_t b);

// size
inlnfunc sui_size_t sui_size(sui_size_type type, f32 value, f32 strictness);
inlnfunc sui_size_t sui_size_pixels(f32 pixels, f32 strictness = 1.0f);
inlnfunc sui_size_t sui_size_percent(f32 percent);
inlnfunc sui_size_t sui_size_by_children(f32 strictness);
inlnfunc sui_size_t sui_size_by_text(f32 padding);

// axis/side/dir
function sui_axis sui_axis_from_dir(sui_dir dir);
function sui_side sui_side_from_dir(sui_dir dir);
function sui_dir sui_dir_from_axis_side(sui_axis axis, sui_side side);

// nodes
function sui_node_t* sui_node_find(sui_key_t key); 
function sui_node_t* sui_node_from_key(sui_node_flags flags, sui_key_t key);
function sui_node_t* sui_node_from_string(sui_node_flags flags, str_t string);
function sui_node_t* sui_node_from_stringf(sui_node_flags flags, char* fmt, ...);

// stacks
function void sui_auto_pop_stacks();

function sui_node_t* sui_top_parent();
function sui_node_t* sui_push_parent(sui_node_t* v);
function sui_node_t* sui_pop_parent();
function sui_node_t* sui_set_next_parent(sui_node_t* v);

function sui_node_flags sui_top_flags();
function sui_node_flags sui_push_flags(sui_node_flags v);
function sui_node_flags sui_pop_flags();
function sui_node_flags sui_set_next_flags(sui_node_flags v);

function sui_node_flags sui_top_omit_flags();
function sui_node_flags sui_push_omit_flags(sui_node_flags v);
function sui_node_flags sui_pop_omit_flags();
function sui_node_flags sui_set_next_omit_flags(sui_node_flags v);

function sui_key_t sui_top_seed_key();
function sui_key_t sui_push_seed_key(sui_key_t v);
function sui_key_t sui_pop_seed_key();
function sui_key_t sui_set_next_seed_key(sui_key_t v);

function sui_key_t sui_top_group_key();
function sui_key_t sui_push_group_key(sui_key_t v);
function sui_key_t sui_pop_group_key();
function sui_key_t sui_set_next_group_key(sui_key_t v);

function str_t sui_top_tag();
function str_t sui_push_tag(str_t v);
function str_t sui_pop_tag();
function str_t sui_set_next_tag(str_t v);

function f32 sui_top_fixed_size_x();
function f32 sui_push_fixed_size_x(f32 v);
function f32 sui_pop_fixed_size_x();
function f32 sui_set_next_fixed_size_x(f32 v);

function f32 sui_top_fixed_size_y();
function f32 sui_push_fixed_size_y(f32 v);
function f32 sui_pop_fixed_size_y();
function f32 sui_set_next_fixed_size_y(f32 v);

function f32 sui_top_fixed_pos_x();
function f32 sui_push_fixed_pos_x(f32 v);
function f32 sui_pop_fixed_pos_x();
function f32 sui_set_next_fixed_pos_x(f32 v);

function f32 sui_top_fixed_pos_y();
function f32 sui_push_fixed_pos_y(f32 v);
function f32 sui_pop_fixed_pos_y();
function f32 sui_set_next_fixed_pos_y(f32 v);

function sui_size_t sui_top_size_x();
function sui_size_t sui_push_size_x(sui_size_t v);
function sui_size_t sui_pop_size_x();
function sui_size_t sui_set_next_size_x(sui_size_t v);

function sui_size_t sui_top_size_y();
function sui_size_t sui_push_size_y(sui_size_t v);
function sui_size_t sui_pop_size_y();
function sui_size_t sui_set_next_size_y(sui_size_t v);

function f32 sui_top_padding_x();
function f32 sui_push_padding_x(f32 v);
function f32 sui_pop_padding_x();
function f32 sui_set_next_padding_x(f32 v);

function f32 sui_top_padding_y();
function f32 sui_push_padding_y(f32 v);
function f32 sui_pop_padding_y();
function f32 sui_set_next_padding_y(f32 v);

function sui_dir sui_top_layout_dir();
function sui_dir sui_push_layout_dir(sui_dir v);
function sui_dir sui_pop_layout_dir();
function sui_dir sui_set_next_layout_dir(sui_dir v);

function sui_text_alignment sui_top_text_alignment();
function sui_text_alignment sui_push_text_alignment(sui_text_alignment v);
function sui_text_alignment sui_pop_text_alignment();
function sui_text_alignment sui_set_next_text_alignment(sui_text_alignment v);

function f32 sui_top_rounding_00();
function f32 sui_push_rounding_00(f32 v);
function f32 sui_pop_rounding_00();
function f32 sui_set_next_rounding_00(f32 v);

function f32 sui_top_rounding_01();
function f32 sui_push_rounding_01(f32 v);
function f32 sui_pop_rounding_01();
function f32 sui_set_next_rounding_01(f32 v);

function f32 sui_top_rounding_10();
function f32 sui_push_rounding_10(f32 v);
function f32 sui_pop_rounding_10();
function f32 sui_set_next_rounding_10(f32 v);

function f32 sui_top_rounding_11();
function f32 sui_push_rounding_11(f32 v);
function f32 sui_pop_rounding_11();
function f32 sui_set_next_rounding_11(f32 v);

function f32 sui_top_border_size();
function f32 sui_push_border_size(f32 v);
function f32 sui_pop_border_size();
function f32 sui_set_next_border_size(f32 v);

function f32 sui_top_shadow_size();
function f32 sui_push_shadow_size(f32 v);
function f32 sui_pop_shadow_size();
function f32 sui_set_next_shadow_size(f32 v);

function gfx_handle_t sui_top_texture();
function gfx_handle_t sui_push_texture(gfx_handle_t v);
function gfx_handle_t sui_pop_texture();
function gfx_handle_t sui_set_next_texture(gfx_handle_t v);

function font_handle_t sui_top_font();
function font_handle_t sui_push_font(font_handle_t v);
function font_handle_t sui_pop_font();
function font_handle_t sui_set_next_font(font_handle_t v);

function f32 sui_top_font_size();
function f32 sui_push_font_size(f32 v);
function f32 sui_pop_font_size();
function f32 sui_set_next_font_size(f32 v);

// tags
function str_t sui_top_tag();
function str_t sui_push_tag(str_t v);
function str_t sui_pop_tag();
function str_t sui_set_next_tag(str_t v);

// group stacks

function void sui_push_size(sui_size_t size_x, sui_size_t size_y);
function void sui_pop_size();
function void sui_set_next_size(sui_size_t size_x, sui_size_t size_y);

function void sui_push_fixed_size(f32 x, f32 y);
function void sui_pop_fixed_size();
function void sui_set_next_fixed_size(f32 x, f32 y);

function void sui_push_fixed_pos(f32 x, f32 y);
function void sui_pop_fixed_pos();
function void sui_set_next_fixed_pos(f32 x, f32 y);

function void sui_push_rect(rect_t rect);
function void sui_pop_rect();
function void sui_set_next_rect(rect_t rect);

function void sui_push_rounding(vec4_t rounding);
function void sui_pop_rounding();
function void sui_set_next_rounding(vec4_t rounding);


#endif // SUI_CORE_H