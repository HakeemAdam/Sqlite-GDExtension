#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "../external/sqlite3/sqlite3.h"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"

class DBManager : public godot::RefCounted{
	GDCLASS(DBManager, godot::RefCounted);


	private:
		sqlite3* db = nullptr;
		static DBManager *singleton;

		godot::Dictionary DBManager::extract_row_to_dict(sqlite3_stmt* stmt, int num_cols);

	protected:
		static void _bind_methods();

	public:
		static DBManager *get_singleton();

		DBManager();
		~DBManager();

		void openDB();

		void create_table();

		void add_note(int64_t p_plant_id, const godot::String& p_title, const godot::String& p_body);

		godot::Dictionary get_note(int64_t p_id);

		godot::Array get_all_notes();

		void update_note(int64_t p_id, const godot::String& p_title, const godot::String& p_body, const godot::String& p_category, const godot::String& p_tags);

		void delete_note(int64_t p_id);

		void add_plant(const godot::String& p_name, const godot::String& p_variety, const godot::String& p_botanical_name);

		void delete_plant(int64_t p_id);

		void update_plant(int64_t p_id, const godot::String& p_name, const godot::String& p_variety, const godot::String& p_botanical_name);

		godot::Dictionary get_plant(int64_t p_id);

		godot::Array get_all_plants();

		godot::Array get_note_by_tag(godot::String p_tag);

		godot::Array get_note_by_category(const godot::String& p_category);

		godot::Array get_note_by_date(const godot::String& p_date_str);

		godot::Array search_notes(const godot::String& p_name);

		godot::Array search_plants(const godot::String& p_name);

		godot::Array get_plants_by_variety(const godot::String& p_variety);

		void closeDB();






};
