#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "spirit.h"

using SpiritId = std::uint64_t;
inline constexpr SpiritId INVALID_SPIRIT_ID = 0;

class Scene : public Spirit {
	protected:
		struct Entry {
				SpiritId id = INVALID_SPIRIT_ID;
				std::unique_ptr<Spirit> spirit;
				bool alive = true;
		};

		std::vector<Entry> _spirits;
		std::vector<Entry> _pending;
		SpiritId _next_id = 1;
		bool _invalid = false;
		float _gravity = 900.0f;

		SpiritId _assign_id();
		Entry *_find(SpiritId id);
		const Entry *_find(SpiritId id) const;
		void _flush_pending();
		void _sweep();

	public:
		SpiritId add_spirit(std::unique_ptr<Spirit> spirit);

		template <typename T, typename... Args>
		SpiritId add_spirit(Args &&...args) {
			return add_spirit(std::make_unique<T>(std::forward<Args>(args)...));
		}

		bool remove_spirit(SpiritId id);
		Spirit *find_spirit(SpiritId id);

		template <typename T>
		T *find_spirit(SpiritId id) {
			return dynamic_cast<T *>(find_spirit(id));
		}

		template <typename T, typename F>
		void each(F &&fn) {
			for (auto &entry : _spirits) {
				if (entry.alive && entry.spirit && !entry.spirit->is_invalid()) {
					if (auto *typed = dynamic_cast<T *>(entry.spirit.get())) {
						fn(*typed);
					}
				}
			}
		}

		// contains / size 为预留接口：目前无调用点
		bool contains(SpiritId id) const;
		std::size_t size() const;
		void clear();
		void purge();

		float gravity() const;
		void set_gravity(float g);

		void finish();

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		void handle_keyboard(key_msg msg) override;
		void handle_mouse(mouse_msg msg) override;
		bool is_invalid() const override;
};
