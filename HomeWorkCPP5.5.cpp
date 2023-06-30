#include <pqxx/pqxx>
#include <iostream>
#include <Windows.h>
#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <memory>

class PostSQL {

public:
	PostSQL() {
		try {

			sql = new pqxx::connection(sql_open);
		}
		catch (std::exception& error) {

			std::cout << error.what() << std::endl;
		}
	}

	~PostSQL() {

		delete sql;
	}

	void create_db() {

		pqxx::work p_sql {*sql};

		p_sql.exec(name);
		p_sql.exec(telephone);
		p_sql.commit();
	}

	void customer_search(std::string name, std::string surname, std::string email = " ") {

		see = false;

		pqxx::work p_sql {*sql};

		typedef std::tuple < std::string, std::string, std::string, std::string> line;

		std::vector < std::tuple<std::string, std::string, std::string, std::string>> search;

		for (const auto& [w_num, w_name, w_surname, w_email] : p_sql.stream< std::string, std::string, std::string, std::string>(
			"select id_name, name, surname, email "
			"from name "
			"where name like '" + p_sql.esc(name) +
			"' and surname like '" + p_sql.esc(surname) + "'")) {

			search.push_back(line(w_num, w_name, w_surname, w_email));
		}

		p_sql.commit();
		
		int j = 0;

		if (search.size() > 0) {

			do {

				std::cout << "В базе найдены совпадения:" << std::endl;

				p_sql.commit();

				for (int i = 0; i < search.size(); i++) {

					std::cout << i << " ";
					std::cout << "Имя фамилия: ";
					std::cout << std::get<1>(search.at(i)) << " ";
					std::cout << std::get<2>(search.at(i)) << ", Email: ";
					std::cout << std::get<3>(search.at(i)) << ". Телефон: ";

					pqxx::work d_sql {*sql};

					for (const auto& [id_telephone, telephone] : d_sql.stream<std::string, std::string>(
						"select id_telephone, telephone "
						"from telephone "
						"where name_id = " + d_sql.esc(std::get<0>(search.at(i))))) {

						std::cout << telephone << " ";
					}

					d_sql.commit();

					std::cout << std::endl;
				}

				std::cout << "Введите № клиента из списка для изменения или -1 для внесение новой записи:";
				std::cin >> j;

				if (j == -1) {
					break;
				}
				else if (j > -1 && j < search.size()) {

					see = true;
					number = std::get<0>(search.at(j));
					p_name = std::get<1>(search.at(j));
					p_surname = std::get<2>(search.at(j));
					p_email = std::get<3>(search.at(j));
				}

			} while (j < 0 || j + 1 > search.size());
		}
	}

	void customer_change(std::string name, std::string surname, std::string email = " ") {

		if (email == " ") {
			email = p_email;
		}

		pqxx::work p_sql {*sql};

		p_sql.exec(
			"update name "
			"set name='" + p_sql.esc(name) + "', "
			"surname='" + p_sql.esc(surname) + "', "
			"email='" + p_sql.esc(email) + "' "
			"where name='" + p_sql.esc(p_name) + "' and surname='" + p_sql.esc(p_surname) + "'");

		p_sql.commit();

		pqxx::work d_sql {*sql};

		for (const auto& [w_name, w_surname, w_email] : d_sql.stream<std::string, std::string, std::string>(
			"select name, surname, email "
			"from name "
			"where id_name = '" + p_sql.esc(number) + "'")) {

			std::cout << "В базу внесены данные: " << w_name << ", " << w_surname << ", " << w_email << "." << std::endl;
		}

		d_sql.commit();
	}

	void creat_client(std::string name, std::string surname, std::string email = "") {

		std::cout << "Добавление клиента в базу." << "\n" << std::endl;

		customer_search(name, surname, email);

		if (see) {

			customer_change(name, surname, email);
		}
		else {

			pqxx::work p_sql {*sql};

			auto num = 0;

			try {

				num = p_sql.query_value<int>("select max(id_name) from name;");
			}
			catch (std::exception& error) {
			}

			number = std::to_string(num + 1);

			p_sql.exec(
				"insert into name (id_name, name, surname, email) "
				" values (" + p_sql.esc(number) + ", '" + p_sql.esc(name) + "', '"
				+ p_sql.esc(surname) + "', '" + p_sql.esc(email) + "')");

			p_sql.commit();

			std::cout << "В базу внесены данные: " << name << ", " << surname << ", " << email << "." << std::endl;
		}

		std::cout << std::endl;
	}

	void creat_phone(std::string telephone) {

		pqxx::work p_sql {*sql};

		auto num = 0;

		try {

			num = p_sql.query_value<int>("select max(id_telephone) from telephone;");
		}
		catch (std::exception& error) {
		}
	
		std::string num_s = std::to_string(num + 1);
		
		p_sql.exec(
			"insert into telephone (id_telephone, name_id, telephone) "
			"values ('" + p_sql.esc(num_s) + "','" + p_sql.esc(number) + "', '" + p_sql.esc(telephone) + "')");

		p_sql.commit();

		std::cout << "В базу для " << p_name << ", " << p_surname <<  " внесен № телефона: " << telephone << "." << std::endl;
	}

	void creat_telephone(std::string name, std::string surname, std::string telephone, std::string email = " ") {

		std::cout << "Добавление номера телефона клиента в базу." << "\n" << std::endl;

		customer_search(name, surname, email);
		creat_phone(telephone);

		std::cout << std::endl;
	}

	void change_client(std::string name, std::string surname, std::string new_name, std::string new_surname, std::string new_email = " ") {

		std::cout << "Изменения данных клиента в базе." << "\n" << std::endl;

		customer_search(name, surname);
		customer_change(new_name, new_surname, new_email);

		std::cout << std::endl;
	}

	void delete_phone() {

		pqxx::work p_sql {*sql};

		int j = 0;

		std::string telephone;

		typedef std::tuple <std::string> line;

		std::vector <std::tuple <std::string>> search;

		for (const auto& w_telephone : p_sql.stream<std::string>(
			"select telephone "
			"from telephone "
			"where name_id = " + p_sql.esc(number))) {

			//line (w_telephone);
			search.push_back(line(w_telephone));
		}
		do {

			std::cout << "Выберите номер телефона для удаления:" << std::endl;

			for (int i = 0; i < search.size(); i++) {

				std::cout << i << " ";
				std::cout << std::get<0>(search.at(i)) << std::endl;
			}

			std::cout << "Введите № телефона из списка или -1 для выхода:";
			std::cin >> j;

			if (j == -1) {
				break;
			}

			telephone = std::get<0>(search.at(j));

		} while (j + 1 > search.size() || j < 0);
		
		p_sql.exec(
			"delete from telephone "
			"where telephone like '" + p_sql.esc(telephone) + "'");

		p_sql.commit();
	}

	void delete_telephone(std::string name, std::string surname) {

		std::cout << "Удаление номера телефона клиента из базы." << "\n" << std::endl;

		customer_search(name, surname);
		delete_phone();

		std::cout << std::endl;
	}

	void delete_customer() {

		pqxx::work p_sql {*sql};

		int j = 2;

		do {
			std::cout << "Подтвердите удаление " << p_name << " " << p_surname << " " << p_email << std::endl;
			std::cout << "Введите 1 для подтверждения, 0 для отмены: ";
			std::cin >> j;
		} while (j < 0 || j > 1);

		if (j == 1) {

			p_sql.exec(
				"delete from telephone "
				"where name_id = " + p_sql.esc(number));

			p_sql.exec(
				"delete from name "
				"where id_name = " + p_sql.esc(number));
		}

		p_sql.commit();
	}

	void delete_client(std::string name, std::string surname) {
	
		std::cout << "Удаление клиента из базы." << "\n" << std::endl;

		customer_search(name, surname);
		delete_customer();

		std::cout << std::endl;
	}

	void search_client(std::string name, std::string surname, std::string email = " ") {

		std::cout << "Поиск клиента по базе." << "\n" << std::endl;

		pqxx::work p_sql {*sql};

		typedef std::tuple < std::string, std::string, std::string, std::string> line;

		std::vector < std::tuple<std::string, std::string, std::string, std::string>> search;

		for (const auto& [w_num, w_name, w_surname, w_email] : p_sql.stream< std::string, std::string, std::string, std::string>(
			"select id_name, name, surname, email "
			"from name "
			"where name like '" + p_sql.esc(name) + "' and surname like '" + p_sql.esc(surname) + "' or email like '" + p_sql.esc(email) + "'")) {

			search.push_back(line(w_num, w_name, w_surname, w_email));
		}

		p_sql.commit();

		for (int i = 0; i < search.size(); i++) {

			std::cout << "Имя фамилия: ";
			std::cout << std::get<1>(search.at(i)) << " ";
			std::cout << std::get<2>(search.at(i)) << ", Email: ";
			std::cout << std::get<3>(search.at(i)) << ". Телефон: ";
			
			pqxx::work d_sql {*sql};

			for (const auto& [id_telephone, telephone] : d_sql.stream<std::string, std::string>(
				"select id_telephone, telephone "
				"from telephone "
				"where name_id = " + d_sql.esc(std::get<0>(search.at(i))))) {

				std::cout << telephone << " ";
			}
			
			d_sql.commit();

			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

	void search_client_phone(std::string telephone) {

		std::cout << "Поиск клиента по номеру в базе." << "\n" << std::endl;

		pqxx::work p_sql {*sql};

		typedef std::tuple < std::string, std::string, std::string, std::string> line;

		std::vector < std::tuple<std::string, std::string, std::string, std::string>> search;

		for (const auto& [w_num, w_name, w_surname, w_email] : p_sql.stream< std::string, std::string, std::string, std::string>(
			"select id_name, name, surname, email "
			"from name n "
			"join telephone t on n.id_name = t.name_id "
			"where t.telephone = '" + p_sql.esc(telephone) + "'")) {

			search.push_back(line(w_num, w_name, w_surname, w_email));
		}

		p_sql.commit();

		for (int i = 0; i < search.size(); i++) {

			std::cout << "Имя фамилия: ";
			std::cout << std::get<1>(search.at(i)) << " ";
			std::cout << std::get<2>(search.at(i)) << ", Email: ";
			std::cout << std::get<3>(search.at(i)) << ". Телефон: ";

			pqxx::work d_sql {*sql};

			for (const auto& [id_telephone, telephone] : d_sql.stream<std::string, std::string>(
				"select id_telephone, telephone "
				"from telephone "
				"where name_id = " + d_sql.esc(std::get<0>(search.at(i))))) {

				std::cout << telephone << " ";
			}

			d_sql.commit();

			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

private:

	bool see = false;
	std::string number;
	std::string p_name;
	std::string p_surname;
	std::string p_email;

	std::string sql_open {
		"host=localhost "
		"port=5432 "
		"dbname=customers "
		"user=postgres "
		"password=pRD(3eW&@?MY$vzS"};

	std::string name {
		"create table if not exists name "
		"(id_name serial primary key, "
		"name text not null, "
		"surname text not null, "
		"email text)"};

	std::string telephone {
		"create table if not exists telephone "
		"(id_telephone serial primary key, "
		"name_id integer references name(id_name), "
		"telephone text not null)"};

	pqxx::connection* sql;
};  



int main() {

	setlocale(LC_ALL, "ru_RU.UFT-8");

	PostSQL posrsql;
	posrsql.create_db();

	try {
		 
		// Добавление клиентов в базу.
		posrsql.creat_client("Виктор", "Калашников");
		posrsql.creat_client("Владимир", "Калашников", "kalashikov@yandex.ru");
		posrsql.creat_client("Сергей", "Иванов", "ivanov@yandex.ru");
		posrsql.creat_client("Дмитрий", "Попов", "1998@gmail.com");
		posrsql.creat_client("Роман", "Волков");

		// Добавление номера телефона.
		posrsql.creat_telephone("Виктор", "Калашников", "48548551");
		posrsql.creat_telephone("Виктор", "Калашников", "59846511");
		posrsql.creat_telephone("Владимир", "Калашников", "48941484");
		posrsql.creat_telephone("Сергей", "Иванов", "75234159");
		posrsql.creat_telephone("Сергей", "Иванов", "48914818");
		posrsql.creat_telephone("Роман", "Волков", "91278881");

		// Изменения данных клиента.
		posrsql.change_client("Сергей", "Иванов", "Михаил", "Иванов");

		// Удаления номера телефона.
		posrsql.delete_telephone("Владимир", "Калашников");

		// Удаления клиента.
		posrsql.delete_client("Виктор", "Калашников");

		// Поиск по Имени и Фамилии.
		posrsql.search_client("Михаил", "Иванов");

		// Поиск по телефону.
		posrsql.search_client_phone("91278881");
	}
	catch (std::exception& error) {

		std::cout << error.what() << std::endl;
	}
}