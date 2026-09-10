#include "db_manager.h"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/char_string.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"

DBManager* DBManager::singleton = nullptr;

void DBManager::_bind_methods(){
	godot::ClassDB::bind_method(godot::D_METHOD("openDB"), &DBManager::openDB);

	godot::ClassDB::bind_method(godot::D_METHOD("create_table"), &DBManager::create_table);

	godot::ClassDB::bind_method(godot::D_METHOD("add_plant","p_name" , "p_variety", "p_botanical_name"), &DBManager::add_plant);

	godot::ClassDB::bind_method(godot::D_METHOD("add_note","p_plant_id" , "p_title", "p_body"), &DBManager::add_note);

	godot::ClassDB::bind_method(godot::D_METHOD("get_note", "p_id"), &DBManager::get_note);

	godot::ClassDB::bind_method(godot::D_METHOD("delete_plant", "p_id"), &DBManager::delete_plant);

	godot::ClassDB::bind_method(godot::D_METHOD("get_plant", "p_id"), &DBManager::get_plant);

	godot::ClassDB::bind_method(godot::D_METHOD("get_all_plants"), &DBManager::get_all_plants);

	godot::ClassDB::bind_method(godot::D_METHOD("get_all_notes"), &DBManager::get_all_notes);

	godot::ClassDB::bind_method(godot::D_METHOD("delete_note", "p_id"), &DBManager::delete_note);

	godot::ClassDB::bind_method(godot::D_METHOD("update_plant", "p_id", "p_name", "p_variety", "p_botanical_name"), &DBManager::update_plant);

	godot::ClassDB::bind_method(godot::D_METHOD("update_note", "p_id", "p_title", "p_body", "p_category", "p_tags"), &DBManager::update_note);

	godot::ClassDB::bind_method(godot::D_METHOD("get_note_by_tag", "p_tag"), &DBManager::get_note_by_tag);

	godot::ClassDB::bind_method(godot::D_METHOD("get_note_by_category", "p_category"), &DBManager::get_note_by_category);

	godot::ClassDB::bind_method(godot::D_METHOD("get_note_by_date", "p_date_str"), &DBManager::get_note_by_date);

	godot::ClassDB::bind_method(godot::D_METHOD("search_notes", "p_name"), &DBManager::search_notes);

	godot::ClassDB::bind_method(godot::D_METHOD("search_plants", "p_name"), &DBManager::search_plants);

	godot::ClassDB::bind_method(godot::D_METHOD("closeDB"), &DBManager::closeDB);


}

DBManager::DBManager(){
	singleton = this;
}

DBManager::~DBManager(){
	singleton = nullptr;
	if (db){
		sqlite3_close(db);
	}
}

DBManager* DBManager::get_singleton(){
	return singleton;
}

void DBManager::openDB(){

	godot::String path = godot::ProjectSettings::get_singleton()->globalize_path("res://");
	godot::String full_path = path + "test.db";

	godot::CharString utf8_path = full_path.utf8();

	int result = sqlite3_open(utf8_path.get_data(), &db);
	if(result != SQLITE_OK){
		godot::print_error("Failed to Open database connection", sqlite3_errmsg(db));
		sqlite3_close(db);
		db = nullptr;
	}else{
		sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
		godot::print_line("opened succesfully");

	}
}

void DBManager::closeDB(){
	if(!db){
		godot::print_error("Databse connection pointer is null", "add_note", __FILE__, __LINE__);
		return;
	}
	sqlite3_close(db);
}

void DBManager::create_table(){
	if (!db){
		godot::print_error("Databse connection pointer is null", "create_table", __FILE__, __LINE__);
		return;
	}

	char* error_message = nullptr;
	const char* query = "CREATE TABLE IF NOT EXISTS plants("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name TEXT NOT NULL UNIQUE,"
			"variety TEXT,"
			"botanical_name TEXT);"

			"CREATE TABLE IF NOT EXISTS notes("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"plant_id INTEGER NOT NULL,"
			"title TEXT NOT NULL,"
			"note TEXT NOT NULL,"
			"created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
			"category TEXT,"
			"tags TEXT,"
			"FOREIGN KEY (plant_id) REFERENCES plants(id) ON DELETE CASCADE)";

	int operation = sqlite3_exec(db, query, NULL, 0, &error_message);

	if (operation != SQLITE_OK){
		 godot::print_error(error_message ? error_message : "Unknown SQLite error", "create_table", __FILE__, __LINE__);
		sqlite3_free(error_message);
	}else{
		godot::print_line("Table notes created");
	}
}

void DBManager::add_plant(const godot::String& p_name, const godot::String& p_variety, const godot::String& p_botanical_name){
	if (!db){
		godot::print_error("Databse connection pointer is null", "add_plant", __FILE__, __LINE__);
		return;
	}
	const char* query = "INSERT INTO plants (name, variety, botanical_name) VALUES (?, ?, ?)";


	sqlite3_stmt* stmt = nullptr;
	int result = sqlite3_prepare_v2(db, query, -1,&stmt, nullptr);

	if (result != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
		return;
	}

	godot::CharString u_name = p_name.utf8();
	godot::CharString u_variety = p_variety.utf8();
	godot::CharString u_botanical = p_botanical_name.utf8();

	sqlite3_bind_text(stmt, 1, u_name.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, u_variety.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, u_botanical.get_data(), -1, SQLITE_TRANSIENT);


	int execution = sqlite3_step(stmt);
	if(execution == SQLITE_DONE){
		godot::print_line("Plant inserted succesfully. ");
	}else{
		godot::print_error("Failed to excute statment:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);


}

void DBManager::delete_plant(int64_t p_id){
	if(!db){
		godot::print_error("Database connection pointer is null", "delete_plant", __FILE__, __LINE__);
		return;
	}

	const char* query = "DELETE FROM plants WHERE id=?";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db,query, -1, &stmt, nullptr);
	if(result != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return;
	}

	sqlite3_bind_int64(stmt, 1, p_id);

	int execution = sqlite3_step(stmt);
	if (execution == SQLITE_DONE) {
		godot::print_line("Plant successfully deleted");
	}else {
		godot::print_error("Failed to delete plant", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);

}

godot::Dictionary DBManager::get_plant(int64_t p_id){
	if (!db){
		godot::print_error("Database connection pointer is null", "get_plant", __FILE__, __LINE__);
		return {};
	}

	godot::Dictionary result = {};
	const char* query = "SELECT * FROM plants WHERE id=?";

	sqlite3_stmt* stmt = nullptr;

	int prepared_stmt = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared_stmt != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
		return{};
	}

	sqlite3_bind_int64(stmt, 1, p_id);

	int execution = sqlite3_step(stmt);
	int num_cols = sqlite3_column_count(stmt);

	godot::print_line("Debug: Status code is ", execution, "| Total colums: ", num_cols);

	if(execution == SQLITE_ROW){
		result = extract_row_to_dict(stmt, num_cols);
	}else if(execution == SQLITE_DONE){
		godot::print_line("No plant found with that ID.");
	}else{
		godot::print_error("Failed to execute SELECT statement:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;

}

godot::Array DBManager::get_all_plants(){
	if(!db){
		godot::print_error("Databse connection pointer is null", "add_note", __FILE__, __LINE__);
		return godot::Array();
	}

	godot::Array result = godot::Array();
	const char* query = "SELECT * FROM plants";

	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
    		return godot::Array();
	}

	int num_cols = sqlite3_column_count(stmt);

	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;
}

void DBManager::update_plant(int64_t p_id, const godot::String& p_name, const godot::String& p_variety, const godot::String& p_botanical_name){
	if (!db){
		godot::print_error("Databse connection pointer is null", "update_note", __FILE__, __LINE__);
		return;
	}

	const char* query = "UPDATE plants SET name = ? , variety = ?, botanical_name = ? WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(result != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
		return;
	}

	godot::CharString u_name = p_name.utf8();
	godot::CharString u_variety = p_variety.utf8();
	godot::CharString u_botanical = p_botanical_name.utf8();

	sqlite3_bind_text(stmt, 1, u_name.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, u_variety.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, u_botanical.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 4, p_id);

	int execution = sqlite3_step(stmt);
	if(execution == SQLITE_DONE){
		godot::print_line("Plant updated succesfully. ");
	}else{
		godot::print_error("Failed to excute statment:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
}

void DBManager::add_note(int64_t p_plant_id, const godot::String& p_title, const godot::String& p_body){
	if (!db){
		godot::print_error("Databse connection pointer is null", "add_note", __FILE__, __LINE__);
		return;
	}

	const char* query = "INSERT INTO notes (plant_id, title, note) VALUES ( ?, ?, ?)";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (result != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
		return;
	}

	godot::CharString u_title = p_title.utf8();
	godot::CharString u_body = p_body.utf8();

	sqlite3_bind_int64(stmt, 1, p_plant_id);
	sqlite3_bind_text(stmt, 2, u_title.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, u_body.get_data(), -1, SQLITE_TRANSIENT);

	int execution = sqlite3_step(stmt);
	if(execution == SQLITE_DONE){
		godot::print_line("Note inserted succesfully. ");
	}else{
		godot::print_error("Failed to add note", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
}

godot::Dictionary DBManager::get_note(int64_t p_id){
	if (!db){
		godot::print_error("Databse connection pointer is null", "get_note", __FILE__, __LINE__);
		return {};
	}

	godot::Dictionary result ={};
	const char* query = "SELECT * FROM notes WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;

	int prepare_res = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (prepare_res != SQLITE_OK) {
    		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
    		return {};
	}

	sqlite3_bind_int64(stmt, 1, p_id);

	int execution = sqlite3_step(stmt);
	int num_cols = sqlite3_column_count(stmt);

	godot::print_line("Debug: Status code is ", execution, "| Total colums: ", num_cols);

	if (execution == SQLITE_ROW) {
        	result = extract_row_to_dict(stmt, num_cols);
	} else if (execution == SQLITE_DONE) {
    		godot::print_line("No note found with that ID.");
	} else {
    		godot::print_error("Failed to execute SELECT statement:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;
}


godot::Array DBManager::get_all_notes(){
	if(!db){
		godot::print_error("Databse connection pointer is null", "add_note", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();
	const char* query = "SELECT * FROM notes";

	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
    		return godot::Array();
	}

	int num_cols = sqlite3_column_count(stmt);

	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;
}

void DBManager::update_note(int64_t p_id, const godot::String& p_title, const godot::String& p_body, const godot::String& p_category, const godot::String& p_tags){
	if (!db){
		godot::print_error("Databse connection pointer is null", "update_note", __FILE__, __LINE__);
		return;
	}

	const char* query = "UPDATE notes SET title = ? , body = ?, category = ?, tags = ? WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(result != SQLITE_OK){
		godot::print_error("Failed to prepare statement", sqlite3_errmsg(db));
		return;
	}

	godot::CharString u_title = p_title.utf8();
	godot::CharString u_body = p_body.utf8();
	godot::CharString u_category = p_category.utf8();
	godot::CharString u_tags = p_tags.utf8();

	sqlite3_bind_text(stmt, 1, u_title.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, u_body.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, u_category.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, u_tags.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 5, p_id);

	int execution = sqlite3_step(stmt);
	if(execution == SQLITE_DONE){
		godot::print_line("Note updated succesfully. ");
	}else{
		godot::print_error("Failed to excute statment:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
}

void DBManager::delete_note(int64_t p_id){
	if(!db){
		godot::print_error("Databse connection pointer is null", "delete_note", __FILE__, __LINE__);
		return;
 	}

	const char* query = "DELETE FROM notes WHERE id=?";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(result != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return;
	}

	sqlite3_bind_int64(stmt, 1, p_id);

	int execution = sqlite3_step(stmt);
	if(execution == SQLITE_DONE){
		godot::print_line("Note successfully deleted");
	}else{
		godot::print_error("Failed to delete note", sqlite3_errmsg(db));
	}
	sqlite3_finalize(stmt);
}

godot::Array DBManager::get_note_by_tag(godot::String p_tag){
	if(!db){
		godot::print_error("Databse connection pointer is null", "get_note_by_tag", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();

	const char* query = "SELECT * FROM notes WHERE tags = ?";
	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return godot::Array();
	}

	godot::CharString u_tag = p_tag.utf8();
	sqlite3_bind_text(stmt, 1, u_tag.get_data(), -1, SQLITE_TRANSIENT);

	int num_cols = sqlite3_column_count(stmt);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;

}

godot::Array DBManager::get_note_by_category(const godot::String& p_category){
	if(!db){
		godot::print_error("Databse connection pointer is null", "get_note_by_category", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();

	const char* query = "SELECT * FROM notes WHERE category = ?";
	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return godot::Array();
	}

	godot::CharString u_category = p_category.utf8();
	sqlite3_bind_text(stmt, 1, u_category.get_data(), -1, SQLITE_TRANSIENT);

	int num_cols = sqlite3_column_count(stmt);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;


}

godot::Array DBManager::get_note_by_date(const godot::String& p_date_str){
	if(!db){
		godot::print_error("Databse connection pointer is null", "get_note_by_date", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();

	const char* query = "SELECT * FROM notes WHERE date(created_on) = date(?);";
	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return godot::Array();
	}

	godot::CharString u_date = p_date_str.utf8();
	sqlite3_bind_text(stmt, 1, u_date.get_data(), -1, SQLITE_TRANSIENT);

	int num_cols = sqlite3_column_count(stmt);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));

	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;
}

godot::Array DBManager::search_notes(const godot::String& p_name){
	if(!db){
		godot::print_error("Databse connection pointer is null", "search_notes", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();

	const char* query = "SELECT * FROM notes WHERE title LIKE ? OR note LIKE ?";
	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return godot::Array();
	}

	godot::String wildcard_search = "%" + p_name + "%";
	godot::CharString u_search = wildcard_search.utf8();

	sqlite3_bind_text(stmt, 1, u_search.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, u_search.get_data(), -1, SQLITE_TRANSIENT);

	int num_cols = sqlite3_column_count(stmt);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;


}

godot::Array DBManager::search_plants(const godot::String& p_name){
	if(!db){
		godot::print_error("Databse connection pointer is null", "search_plants", __FILE__, __LINE__);
		return godot::Array();
 	}

	godot::Array result = godot::Array();

	const char* query = "SELECT * FROM plants WHERE name LIKE ? OR variety LIKE ?";
	sqlite3_stmt* stmt = nullptr;

	int prepared = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if(prepared != SQLITE_OK){
		godot::print_error("Failed to prepare statement" ,sqlite3_errmsg(db));
		return godot::Array();
	}

	godot::String wildcard_search = "%" + p_name + "%";
	godot::CharString u_search = wildcard_search.utf8();

	sqlite3_bind_text(stmt, 1, u_search.get_data(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, u_search.get_data(), -1, SQLITE_TRANSIENT);

	int num_cols = sqlite3_column_count(stmt);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		result.push_back(extract_row_to_dict(stmt, num_cols));
	}

	int final_status = sqlite3_errcode(db);
	if (final_status != SQLITE_OK && final_status != SQLITE_DONE) {
    		godot::print_error("Loop terminated early due to a database error:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return result;



}


godot::Dictionary DBManager::extract_row_to_dict(sqlite3_stmt* stmt, int num_cols) {
    godot::Dictionary row = {};
    for (int i = 0; i < num_cols; i++) {
        const char* col_name = sqlite3_column_name(stmt, i);
        int col_type = sqlite3_column_type(stmt, i);

        switch (col_type) {
            case SQLITE_INTEGER:
                row[col_name] = static_cast<int64_t>(sqlite3_column_int64(stmt, i));
                break;
            case SQLITE_TEXT: {
                const unsigned char* field_text = sqlite3_column_text(stmt, i);
                row[col_name] = field_text ? godot::String(reinterpret_cast<const char*>(field_text)) : godot::String("");
                break;
            }
            case SQLITE_NULL:
            default:
                row[col_name] = godot::Variant();
                break;
        }
    }
    return row;
}
