/*************************************************************************/
/*  animated_sprite_2d.cpp                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "animated_sprite_2d.h"

#include "core/os/os.h"
#include "scene/scene_string_names.h"

#define NORMAL_SUFFIX "_normal"
#define SPECULAR_SUFFIX "_specular"

#ifdef TOOLS_ENABLED
Dictionary AnimatedSprite2D::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	return state;
}

void AnimatedSprite2D::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
}

void AnimatedSprite2D::_edit_set_pivot(const Point2 &p_pivot) {
	set_offset(get_offset() - p_pivot);
	set_position(get_transform().xform(p_pivot));
}

Point2 AnimatedSprite2D::_edit_get_pivot() const {
	return Vector2();
}

bool AnimatedSprite2D::_edit_use_pivot() const {
	return true;
}

Rect2 AnimatedSprite2D::_edit_get_rect() const {
	return _get_rect();
}

bool AnimatedSprite2D::_edit_use_rect() const {
	if (!frames.is_valid() || !frames->has_animation(animation) || frame < 0 || frame >= frames->get_frame_count(animation)) {
		return false;
	}
	Ref<Texture2D> t;
	if (animation)
		t = frames->get_frame(animation, frame);
	return t.is_valid();
}
#endif

Rect2 AnimatedSprite2D::get_anchorable_rect() const {
	return _get_rect();
}

Rect2 AnimatedSprite2D::_get_rect() const {
	if (!frames.is_valid() || !frames->has_animation(animation) || frame < 0 || frame >= frames->get_frame_count(animation)) {
		return Rect2();
	}

	Ref<Texture2D> t;
	if (animation)
		t = frames->get_frame(animation, frame);
	if (t.is_null())
		return Rect2();
	Size2 s = t->get_size();

	Point2 ofs = offset;
	if (centered)
		ofs -= Size2(s) / 2;

	if (s == Size2(0, 0))
		s = Size2(1, 1);

	return Rect2(ofs, s);
}

void SpriteFrames::add_frame(const StringName &p_anim, const Ref<Texture2D> &p_frame, int p_at_pos) {

	Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_MSG(!E, "Animation '" + String(p_anim) + "' doesn't exist.");

	if (p_at_pos >= 0 && p_at_pos < E->get().frames.size())
		E->get().frames.insert(p_at_pos, p_frame);
	else
		E->get().frames.push_back(p_frame);

	emit_changed();
}

int SpriteFrames::get_frame_count(const StringName &p_anim) const {
	const Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_V_MSG(!E, 0, "Animation '" + String(p_anim) + "' doesn't exist.");

	return E->get().frames.size();
}

void SpriteFrames::remove_frame(const StringName &p_anim, int p_idx) {

	Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_MSG(!E, "Animation '" + String(p_anim) + "' doesn't exist.");

	E->get().frames.remove(p_idx);
	emit_changed();
}
void SpriteFrames::clear(const StringName &p_anim) {

	Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_MSG(!E, "Animation '" + String(p_anim) + "' doesn't exist.");

	E->get().frames.clear();
	emit_changed();
}

void SpriteFrames::clear_all() {

	animations.clear();
	add_animation("default");
}

void SpriteFrames::add_animation(const StringName &p_anim) {

	ERR_FAIL_COND_MSG(animations.has(p_anim), "SpriteFrames already has animation '" + p_anim + "'.");

	animations[p_anim] = Anim();
	animations[p_anim].normal_name = String(p_anim) + NORMAL_SUFFIX;
	animations[p_anim].specular_name = String(p_anim) + SPECULAR_SUFFIX;
}

bool SpriteFrames::has_animation(const StringName &p_anim) const {

	return animations.has(p_anim);
}
void SpriteFrames::remove_animation(const StringName &p_anim) {

	animations.erase(p_anim);
}

void SpriteFrames::rename_animation(const StringName &p_prev, const StringName &p_next) {

	ERR_FAIL_COND_MSG(!animations.has(p_prev), "SpriteFrames doesn't have animation '" + String(p_prev) + "'.");
	ERR_FAIL_COND_MSG(animations.has(p_next), "Animation '" + String(p_next) + "' already exists.");

	Anim anim = animations[p_prev];
	animations.erase(p_prev);
	animations[p_next] = anim;
	animations[p_next].normal_name = String(p_next) + NORMAL_SUFFIX;
	animations[p_next].specular_name = String(p_next) + SPECULAR_SUFFIX;
}

Vector<String> SpriteFrames::_get_animation_list() const {

	Vector<String> ret;
	List<StringName> al;
	get_animation_list(&al);
	for (List<StringName>::Element *E = al.front(); E; E = E->next()) {

		ret.push_back(E->get());
	}

	return ret;
}

void SpriteFrames::get_animation_list(List<StringName> *r_animations) const {

	for (const Map<StringName, Anim>::Element *E = animations.front(); E; E = E->next()) {
		r_animations->push_back(E->key());
	}
}

Vector<String> SpriteFrames::get_animation_names() const {

	Vector<String> names;
	for (const Map<StringName, Anim>::Element *E = animations.front(); E; E = E->next()) {
		names.push_back(E->key());
	}
	names.sort();
	return names;
}

void SpriteFrames::set_animation_speed(const StringName &p_anim, float p_fps) {

	ERR_FAIL_COND_MSG(p_fps < 0, "Animation speed cannot be negative (" + itos(p_fps) + ").");
	Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_MSG(!E, "Animation '" + String(p_anim) + "' doesn't exist.");
	E->get().speed = p_fps;
}
float SpriteFrames::get_animation_speed(const StringName &p_anim) const {

	const Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_V_MSG(!E, 0, "Animation '" + String(p_anim) + "' doesn't exist.");
	return E->get().speed;
}

void SpriteFrames::set_animation_loop(const StringName &p_anim, bool p_loop) {
	Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_MSG(!E, "Animation '" + String(p_anim) + "' doesn't exist.");
	E->get().loop = p_loop;
}
bool SpriteFrames::get_animation_loop(const StringName &p_anim) const {
	const Map<StringName, Anim>::Element *E = animations.find(p_anim);
	ERR_FAIL_COND_V_MSG(!E, false, "Animation '" + String(p_anim) + "' doesn't exist.");
	return E->get().loop;
}

void SpriteFrames::_set_frames(const Array &p_frames) {

	clear_all();
	Map<StringName, Anim>::Element *E = animations.find(SceneStringNames::get_singleton()->_default);
	ERR_FAIL_COND(!E);

	E->get().frames.resize(p_frames.size());
	for (int i = 0; i < E->get().frames.size(); i++)
		E->get().frames.write[i] = p_frames[i];
}
Array SpriteFrames::_get_frames() const {

	return Array();
}

Array SpriteFrames::_get_animations() const {

	Array anims;
	for (Map<StringName, Anim>::Element *E = animations.front(); E; E = E->next()) {
		Dictionary d;
		d["name"] = E->key();
		d["speed"] = E->get().speed;
		d["loop"] = E->get().loop;
		Array frames;
		for (int i = 0; i < E->get().frames.size(); i++) {
			frames.push_back(E->get().frames[i]);
		}
		d["frames"] = frames;
		anims.push_back(d);
	}

	return anims;
}
void SpriteFrames::_set_animations(const Array &p_animations) {

	animations.clear();
	for (int i = 0; i < p_animations.size(); i++) {

		Dictionary d = p_animations[i];

		ERR_CONTINUE(!d.has("name"));
		ERR_CONTINUE(!d.has("speed"));
		ERR_CONTINUE(!d.has("loop"));
		ERR_CONTINUE(!d.has("frames"));

		Anim anim;
		anim.speed = d["speed"];
		anim.loop = d["loop"];
		Array frames = d["frames"];
		for (int j = 0; j < frames.size(); j++) {

			RES res = frames[j];
			anim.frames.push_back(res);
		}

		animations[d["name"]] = anim;
	}
}

void SpriteFrames::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_animation", "anim"), &SpriteFrames::add_animation);
	ClassDB::bind_method(D_METHOD("has_animation", "anim"), &SpriteFrames::has_animation);
	ClassDB::bind_method(D_METHOD("remove_animation", "anim"), &SpriteFrames::remove_animation);
	ClassDB::bind_method(D_METHOD("rename_animation", "anim", "newname"), &SpriteFrames::rename_animation);

	ClassDB::bind_method(D_METHOD("get_animation_names"), &SpriteFrames::get_animation_names);

	ClassDB::bind_method(D_METHOD("set_animation_speed", "anim", "speed"), &SpriteFrames::set_animation_speed);
	ClassDB::bind_method(D_METHOD("get_animation_speed", "anim"), &SpriteFrames::get_animation_speed);

	ClassDB::bind_method(D_METHOD("set_animation_loop", "anim", "loop"), &SpriteFrames::set_animation_loop);
	ClassDB::bind_method(D_METHOD("get_animation_loop", "anim"), &SpriteFrames::get_animation_loop);

	ClassDB::bind_method(D_METHOD("add_frame", "anim", "frame", "at_position"), &SpriteFrames::add_frame, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_frame_count", "anim"), &SpriteFrames::get_frame_count);
	ClassDB::bind_method(D_METHOD("get_frame", "anim", "idx"), &SpriteFrames::get_frame);
	ClassDB::bind_method(D_METHOD("set_frame", "anim", "idx", "txt"), &SpriteFrames::set_frame);
	ClassDB::bind_method(D_METHOD("remove_frame", "anim", "idx"), &SpriteFrames::remove_frame);
	ClassDB::bind_method(D_METHOD("clear", "anim"), &SpriteFrames::clear);
	ClassDB::bind_method(D_METHOD("clear_all"), &SpriteFrames::clear_all);

	ClassDB::bind_method(D_METHOD("_set_frames"), &SpriteFrames::_set_frames);
	ClassDB::bind_method(D_METHOD("_get_frames"), &SpriteFrames::_get_frames);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "frames", PROPERTY_HINT_NONE, "", 0), "_set_frames", "_get_frames"); //compatibility

	ClassDB::bind_method(D_METHOD("_set_animations"), &SpriteFrames::_set_animations);
	ClassDB::bind_method(D_METHOD("_get_animations"), &SpriteFrames::_get_animations);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "animations", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_animations", "_get_animations"); //compatibility
}

SpriteFrames::SpriteFrames() {

	add_animation(SceneStringNames::get_singleton()->_default);
}

void AnimatedSprite2D::_validate_property(PropertyInfo &property) const {

	if (!frames.is_valid())
		return;
	if (property.name == "animation") {

		property.hint = PROPERTY_HINT_ENUM;
		List<StringName> names;
		frames->get_animation_list(&names);
		names.sort_custom<StringName::AlphCompare>();

		bool current_found = false;

		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {
			if (E->prev()) {
				property.hint_string += ",";
			}

			property.hint_string += String(E->get());
			if (animation == E->get()) {
				current_found = true;
			}
		}

		if (!current_found) {
			if (property.hint_string == String()) {
				property.hint_string = String(animation);
			} else {
				property.hint_string = String(animation) + "," + property.hint_string;
			}
		}
	}

	if (property.name == "frame") {
		property.hint = PROPERTY_HINT_RANGE;
		if (frames->has_animation(animation) && frames->get_frame_count(animation) > 1) {
			property.hint_string = "0," + itos(frames->get_frame_count(animation) - 1) + ",1";
		}
		property.usage |= PROPERTY_USAGE_KEYING_INCREMENTS;
	}
}

void AnimatedSprite2D::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {

			if (frames.is_null())
				return;
			if (!frames->has_animation(animation))
				return;
			if (frame < 0)
				return;

			float speed = frames->get_animation_speed(animation) * speed_scale;
			if (speed == 0)
				return; //do nothing

			float remaining = get_process_delta_time();

			while (remaining) {

				if (timeout <= 0) {

					timeout = _get_frame_duration();

					int fc = frames->get_frame_count(animation);
					if ((!backwards && frame >= fc - 1) || (backwards && frame <= 0)) {
						if (frames->get_animation_loop(animation)) {
							if (backwards)
								frame = fc - 1;
							else
								frame = 0;

							emit_signal(SceneStringNames::get_singleton()->animation_finished);
						} else {
							if (backwards)
								frame = 0;
							else
								frame = fc - 1;

							if (!is_over) {
								is_over = true;
								emit_signal(SceneStringNames::get_singleton()->animation_finished);
							}
						}
					} else {
						if (backwards)
							frame--;
						else
							frame++;
					}

					update();
					_change_notify("frame");
					emit_signal(SceneStringNames::get_singleton()->frame_changed);
				}

				float to_process = MIN(timeout, remaining);
				remaining -= to_process;
				timeout -= to_process;
			}
		} break;

		case NOTIFICATION_DRAW: {

			if (frames.is_null())
				return;
			if (frame < 0)
				return;
			if (!frames->has_animation(animation))
				return;

			Ref<Texture2D> texture = frames->get_frame(animation, frame);
			if (texture.is_null())
				return;

			Ref<Texture2D> normal = frames->get_normal_frame(animation, frame);
			Ref<Texture2D> specular = frames->get_specular_frame(animation, frame);

			RID ci = get_canvas_item();

			Size2i s;
			s = texture->get_size();
			Point2 ofs = offset;
			if (centered)
				ofs -= s / 2;

			if (Engine::get_singleton()->get_use_pixel_snap()) {
				ofs = ofs.floor();
			}
			Rect2 dst_rect(ofs, s);

			if (hflip)
				dst_rect.size.x = -dst_rect.size.x;
			if (vflip)
				dst_rect.size.y = -dst_rect.size.y;

			texture->draw_rect_region(ci, dst_rect, Rect2(Vector2(), texture->get_size()), Color(1, 1, 1), false, normal, specular, Color(specular_color.r, specular_color.g, specular_color.b, shininess));

		} break;
	}
}

void AnimatedSprite2D::set_sprite_frames(const Ref<SpriteFrames> &p_frames) {

	if (frames.is_valid())
		frames->disconnect("changed", callable_mp(this, &AnimatedSprite2D::_res_changed));
	frames = p_frames;
	if (frames.is_valid())
		frames->connect("changed", callable_mp(this, &AnimatedSprite2D::_res_changed));

	if (!frames.is_valid()) {
		frame = 0;
	} else {
		set_frame(frame);
	}

	_change_notify();
	_reset_timeout();
	update();
	update_configuration_warning();
}

Ref<SpriteFrames> AnimatedSprite2D::get_sprite_frames() const {

	return frames;
}

void AnimatedSprite2D::set_frame(int p_frame) {

	if (!frames.is_valid()) {
		return;
	}

	if (frames->has_animation(animation)) {
		int limit = frames->get_frame_count(animation);
		if (p_frame >= limit)
			p_frame = limit - 1;
	}

	if (p_frame < 0)
		p_frame = 0;

	if (frame == p_frame)
		return;

	frame = p_frame;
	_reset_timeout();
	update();
	_change_notify("frame");
	emit_signal(SceneStringNames::get_singleton()->frame_changed);
}
int AnimatedSprite2D::get_frame() const {

	return frame;
}

void AnimatedSprite2D::set_speed_scale(float p_speed_scale) {

	float elapsed = _get_frame_duration() - timeout;

	speed_scale = MAX(p_speed_scale, 0.0f);

	// We adapt the timeout so that the animation speed adapts as soon as the speed scale is changed
	_reset_timeout();
	timeout -= elapsed;
}

float AnimatedSprite2D::get_speed_scale() const {

	return speed_scale;
}

void AnimatedSprite2D::set_centered(bool p_center) {

	centered = p_center;
	update();
	item_rect_changed();
}

bool AnimatedSprite2D::is_centered() const {

	return centered;
}

void AnimatedSprite2D::set_offset(const Point2 &p_offset) {

	offset = p_offset;
	update();
	item_rect_changed();
	_change_notify("offset");
}
Point2 AnimatedSprite2D::get_offset() const {

	return offset;
}

void AnimatedSprite2D::set_flip_h(bool p_flip) {

	hflip = p_flip;
	update();
}
bool AnimatedSprite2D::is_flipped_h() const {

	return hflip;
}

void AnimatedSprite2D::set_flip_v(bool p_flip) {

	vflip = p_flip;
	update();
}
bool AnimatedSprite2D::is_flipped_v() const {

	return vflip;
}

void AnimatedSprite2D::_res_changed() {

	set_frame(frame);
	_change_notify("frame");
	_change_notify("animation");
	update();
}

void AnimatedSprite2D::_set_playing(bool p_playing) {

	if (playing == p_playing)
		return;
	playing = p_playing;
	_reset_timeout();
	set_process_internal(playing);
}

bool AnimatedSprite2D::_is_playing() const {

	return playing;
}

void AnimatedSprite2D::play(const StringName &p_animation, const bool p_backwards) {

	backwards = p_backwards;

	if (p_animation) {
		set_animation(p_animation);
		if (backwards && get_frame() == 0)
			set_frame(frames->get_frame_count(p_animation) - 1);
	}

	_set_playing(true);
}

void AnimatedSprite2D::stop() {

	_set_playing(false);
}

bool AnimatedSprite2D::is_playing() const {

	return playing;
}

float AnimatedSprite2D::_get_frame_duration() {
	if (frames.is_valid() && frames->has_animation(animation)) {
		float speed = frames->get_animation_speed(animation) * speed_scale;
		if (speed > 0) {
			return 1.0 / speed;
		}
	}
	return 0.0;
}

void AnimatedSprite2D::_reset_timeout() {

	if (!playing)
		return;

	timeout = _get_frame_duration();
	is_over = false;
}

void AnimatedSprite2D::set_animation(const StringName &p_animation) {

	ERR_FAIL_COND_MSG(frames == nullptr, vformat("There is no animation with name '%s'.", p_animation));
	ERR_FAIL_COND_MSG(frames->get_animation_names().find(p_animation) == -1, vformat("There is no animation with name '%s'.", p_animation));

	if (animation == p_animation)
		return;

	animation = p_animation;
	_reset_timeout();
	set_frame(0);
	_change_notify();
	update();
}
StringName AnimatedSprite2D::get_animation() const {

	return animation;
}

String AnimatedSprite2D::get_configuration_warning() const {

	if (frames.is_null()) {
		return TTR("A SpriteFrames resource must be created or set in the \"Frames\" property in order for AnimatedSprite to display frames.");
	}

	return String();
}

void AnimatedSprite2D::set_specular_color(const Color &p_color) {
	specular_color = p_color;
	update();
}

Color AnimatedSprite2D::get_specular_color() const {
	return specular_color;
}

void AnimatedSprite2D::set_shininess(float p_shininess) {
	shininess = CLAMP(p_shininess, 0.0, 1.0);
	update();
}

float AnimatedSprite2D::get_shininess() const {
	return shininess;
}

void AnimatedSprite2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_sprite_frames", "sprite_frames"), &AnimatedSprite2D::set_sprite_frames);
	ClassDB::bind_method(D_METHOD("get_sprite_frames"), &AnimatedSprite2D::get_sprite_frames);

	ClassDB::bind_method(D_METHOD("set_animation", "animation"), &AnimatedSprite2D::set_animation);
	ClassDB::bind_method(D_METHOD("get_animation"), &AnimatedSprite2D::get_animation);

	ClassDB::bind_method(D_METHOD("_set_playing", "playing"), &AnimatedSprite2D::_set_playing);
	ClassDB::bind_method(D_METHOD("_is_playing"), &AnimatedSprite2D::_is_playing);

	ClassDB::bind_method(D_METHOD("play", "anim", "backwards"), &AnimatedSprite2D::play, DEFVAL(StringName()), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("stop"), &AnimatedSprite2D::stop);
	ClassDB::bind_method(D_METHOD("is_playing"), &AnimatedSprite2D::is_playing);

	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &AnimatedSprite2D::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &AnimatedSprite2D::is_centered);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &AnimatedSprite2D::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &AnimatedSprite2D::get_offset);

	ClassDB::bind_method(D_METHOD("set_flip_h", "flip_h"), &AnimatedSprite2D::set_flip_h);
	ClassDB::bind_method(D_METHOD("is_flipped_h"), &AnimatedSprite2D::is_flipped_h);

	ClassDB::bind_method(D_METHOD("set_flip_v", "flip_v"), &AnimatedSprite2D::set_flip_v);
	ClassDB::bind_method(D_METHOD("is_flipped_v"), &AnimatedSprite2D::is_flipped_v);

	ClassDB::bind_method(D_METHOD("set_frame", "frame"), &AnimatedSprite2D::set_frame);
	ClassDB::bind_method(D_METHOD("get_frame"), &AnimatedSprite2D::get_frame);

	ClassDB::bind_method(D_METHOD("set_speed_scale", "speed_scale"), &AnimatedSprite2D::set_speed_scale);
	ClassDB::bind_method(D_METHOD("get_speed_scale"), &AnimatedSprite2D::get_speed_scale);

	ClassDB::bind_method(D_METHOD("set_specular_color", "color"), &AnimatedSprite2D::set_specular_color);
	ClassDB::bind_method(D_METHOD("get_specular_color"), &AnimatedSprite2D::get_specular_color);

	ClassDB::bind_method(D_METHOD("set_shininess", "shininess"), &AnimatedSprite2D::set_shininess);
	ClassDB::bind_method(D_METHOD("get_shininess"), &AnimatedSprite2D::get_shininess);

	ADD_SIGNAL(MethodInfo("frame_changed"));
	ADD_SIGNAL(MethodInfo("animation_finished"));

	ADD_GROUP("Animation", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "frames", PROPERTY_HINT_RESOURCE_TYPE, "SpriteFrames"), "set_sprite_frames", "get_sprite_frames");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "animation"), "set_animation", "get_animation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "set_frame", "get_frame");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed_scale"), "set_speed_scale", "get_speed_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing"), "_set_playing", "_is_playing");
	ADD_GROUP("Lighting", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "specular_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_specular_color", "get_specular_color");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shininess", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_shininess", "get_shininess");
	ADD_GROUP("Offset", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_h"), "set_flip_h", "is_flipped_h");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_v"), "set_flip_v", "is_flipped_v");
}

AnimatedSprite2D::AnimatedSprite2D() {

	centered = true;
	hflip = false;
	vflip = false;

	frame = 0;
	speed_scale = 1.0f;
	playing = false;
	backwards = false;
	animation = "default";
	timeout = 0;
	is_over = false;
	specular_color = Color(1, 1, 1, 1);
	shininess = 1.0;
}
