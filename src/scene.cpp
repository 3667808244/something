#include "scene.h"

#include <algorithm>
#include <utility>

SpiritId Scene::_assign_id() { return _next_id++; }

Scene::Entry *Scene::_find(SpiritId id) {
	for (auto &entry : _spirits) {
		if (entry.id == id) {
			return &entry;
		}
	}
	for (auto &entry : _pending) {
		if (entry.id == id) {
			return &entry;
		}
	}
	return nullptr;
}

const Scene::Entry *Scene::_find(SpiritId id) const {
	for (const auto &entry : _spirits) {
		if (entry.id == id) {
			return &entry;
		}
	}
	for (const auto &entry : _pending) {
		if (entry.id == id) {
			return &entry;
		}
	}
	return nullptr;
}

void Scene::_flush_pending() {
	for (auto &entry : _pending) {
		if (entry.alive) {
			_spirits.push_back(std::move(entry));
		}
	}
	_pending.clear();
}

void Scene::_sweep() {
	auto dead = [](const Entry &entry) { return !entry.alive || !entry.spirit || entry.spirit->is_invalid(); };
	_spirits.erase(std::remove_if(_spirits.begin(), _spirits.end(), dead), _spirits.end());
	_pending.erase(std::remove_if(_pending.begin(), _pending.end(), dead), _pending.end());
}

SpiritId Scene::add_spirit(std::unique_ptr<Spirit> spirit) {
	Entry entry;
	entry.id = _assign_id();
	entry.spirit = std::move(spirit);
	entry.alive = true;
	SpiritId id = entry.id;
	_pending.push_back(std::move(entry));
	return id;
}

bool Scene::remove_spirit(SpiritId id) {
	Entry *entry = _find(id);
	if (entry == nullptr || !entry->alive) {
		return false;
	}
	entry->alive = false;
	return true;
}

Spirit *Scene::find_spirit(SpiritId id) {
	Entry *entry = _find(id);
	if (entry == nullptr || !entry->alive) {
		return nullptr;
	}
	return entry->spirit.get();
}

bool Scene::contains(SpiritId id) const {
	const Entry *entry = _find(id);
	return entry != nullptr && entry->alive;
}

std::size_t Scene::size() const {
	std::size_t count = 0;
	for (const auto &entry : _spirits) {
		if (entry.alive && entry.spirit) {
			++count;
		}
	}
	for (const auto &entry : _pending) {
		if (entry.alive && entry.spirit) {
			++count;
		}
	}
	return count;
}

void Scene::clear() {
	for (auto &entry : _spirits) {
		entry.alive = false;
	}
	for (auto &entry : _pending) {
		entry.alive = false;
	}
}

void Scene::purge() {
	_flush_pending();
	_sweep();
}

float Scene::gravity() const { return _gravity; }

void Scene::set_gravity(float g) { _gravity = g; }

void Scene::finish() { _invalid = true; }

void Scene::draw(PIMAGE image) {
	for (auto &entry : _spirits) {
		if (entry.alive && entry.spirit && !entry.spirit->is_invalid()) {
			entry.spirit->draw(image);
		}
	}
}

void Scene::update(float delta, Scene &scene [[maybe_unused]]) {
	_flush_pending();
	for (auto &entry : _spirits) {
		if (entry.alive && entry.spirit && !entry.spirit->is_invalid()) {
			entry.spirit->update(delta, *this);
		}
	}
	_sweep();
}

void Scene::handal_keybord(key_msg msg) {
	for (auto &entry : _spirits) {
		if (entry.alive && entry.spirit && !entry.spirit->is_invalid()) {
			entry.spirit->handal_keybord(msg);
		}
	}
}

void Scene::handal_mouse(mouse_msg msg) {
	for (auto &entry : _spirits) {
		if (entry.alive && entry.spirit && !entry.spirit->is_invalid()) {
			entry.spirit->handal_mouse(msg);
		}
	}
}

bool Scene::is_invalid() const { return _invalid; }
