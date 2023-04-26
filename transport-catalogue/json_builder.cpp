#include "json_builder.h"

#include <stdexcept>
#include <utility>

using namespace std::literals;


// ОПИСАНИЕ

// ОБРАБОТКА ОШИБОК
//	В случае использования методов в неверном контексте ваш код должен выбросить
//	исключение типа std::logic_error с понятным сообщением об ошибке.
//	Это должно происходить в следующих ситуациях :
//	  - Вызов метода Build при неготовом описываемом объекте, то есть сразу после
//		конструктора или при незаконченных массивах и словарях.
//    - Вызов любого метода, кроме Build, при готовом объекте.
//	  - Вызов метода Key снаружи словаря или сразу после другого Key.
//	  - Вызов Value, StartDict или StartArray где-либо, кроме как после конструктора,
//		после Key или после предыдущего элемента массива.
//	  - Вызов EndDict или EndArray в контексте другого контейнера

namespace json {

	// При определении словаря задаёт строковое значение ключа
	// для очередной пары ключ-значение. Следующий вызов метода обязательно должен
	// задавать соответствующее этому ключу значение с помощью метода Value или
	// начинать его определение с помощью StartDict или StartArray.
	AfterKey Builder::Key(std::string key) {
		// Проверка вызова снаружи словаря или сразу после другого Key
		if (node_stack_.empty() && !root_.IsDict()) {
			throw std::logic_error("\"Key\": newly created object isn't dictionary."s);
		}

		if (!node_stack_.empty() && !node_stack_.back()->IsDict()) {
			throw std::logic_error("\"Key\": called ouside of dictionary."s);
		}

		if (key_is_entered_) {
			throw std::logic_error("\"Key\": called after another \"Key\"."s);
		}

		key_buffer_ = std::move(key);
		key_is_entered_ = true;

		return { *this };
	}


	// Задаёт значение, соответствующее ключу при определении словаря, очередной
	// элемент массива или, если вызвать сразу после конструктора json::Builder,
	// всё содержимое конструируемого JSON - объекта.Может принимать как простой
	// объект — число или строку — так и целый массив или словарь.
	// Здесь Node::Value — это синоним для базового класса Node, шаблона variant
	// с набором возможных типов - значений.Смотрите заготовку кода.
	AfterNoLim Builder::Value(Node::Value value) {

		// Проверка, что объект готов: задан root_ и массив создания пуст
		if (ObjectIsFinished()) {
			throw std::logic_error("\"Value\": called for finished object."s);
		}

		if (!CheckCorrectCall()) {
			throw std::logic_error("\"Value\": called in wrong place."s);
		}

		// Создание value сразу после конструктора
		if (ObjectJustCreated()) {
			root_ = std::move(Node(std::move(value)));

			return { *this };
		}

		// Привязка к последнему узлу в node_stack_
		auto& last_node = *node_stack_.back();

		if (last_node.IsArray()) {
			// Возврат массива из узла
			Array buffer_array = last_node.AsArray();

			// Внесение Value в буфферный массив
			buffer_array.push_back(std::move(Node(std::move(value))));

			// Присвоение нового узла для root_
			last_node = std::move(Node(std::move(buffer_array)));

			return { *this };
		}
		else if (last_node.IsDict()) {
			Dict buffer_dict = last_node.AsDict();

			buffer_dict[key_buffer_] = std::move(Node(std::move(value)));
			//key_buffer_.erase();
			key_is_entered_ = false;

			last_node = std::move(Node(std::move(buffer_dict)));

			return { *this };
		}
		else {
			throw std::logic_error("\"Value\": incorrect call for not empty node_stack_"s);
		}

		throw std::logic_error("\"Value\": wasn't added"s);
	}

	// Начинает определение сложного значения - словаря.Вызывается в тех же контекстах,
	// что и Value. Следующим вызовом обязательно должен быть Key или EndDict.
	AfterStartDict Builder::StartDict() {
		// Проверка, что объект готов: задан root_ и массив создания пуст
		if (ObjectIsFinished()) {
			throw std::logic_error("\"StartDict\": called for finished object."s);
		}

		if (!CheckCorrectCall()) {
			throw std::logic_error("\"StartDict\": called in wrong place."s);
		}

		// Узел с пустым словарем
		Node empty_dict_n(Dict{});

		if (!StartList(std::move(empty_dict_n))) {
			throw std::logic_error("\"StartDict\": wasn't started"s);
		}

		return { *this };
	}

	// Начинает определение сложного значения - массива.Вызывается в тех же контекстах,
	// что и Value.Следующим вызовом обязательно должен быть EndArray или любой,
	// задающий новое значение : Value, StartDict или StartArray.
	AfterStartArray Builder::StartArray() {
		// Проверка, что объект готов: задан root_ и массив создания пуст
		if (ObjectIsFinished()) {
			throw std::logic_error("\"StartArray\": called for finished object."s);
		}

		if (!CheckCorrectCall()) {
			throw std::logic_error("\"StartArray\": called in wrong place."s);
		}

		// Узел с пустым списком
		Node empty_array_n(Array{});


		if (!StartList(std::move(empty_array_n))) {
			throw std::logic_error("\"StartArray\": wasn't started"s);
		}

		return { *this };
	}

	// Завершает определение сложного значения-словаря. Последним незавершённым
	// вызовом Start* должен быть StartDict.
	AfterNoLim Builder::EndDict() {
		// Что объект не только что создан
		if (ObjectJustCreated()) {
			throw std::logic_error("\"EndDict\": called for empty object"s);
		}

		// Проверка, что корневой узел словарь, если стек пуст, или последний узел в стеке - словарь
		if ((node_stack_.empty() && !root_.IsDict()) || (!node_stack_.empty() && !node_stack_.back()->IsDict())) {
			throw std::logic_error("\"EndDict\": called for not dict"s);
		}

		// Проверка, что нет незаполненных ключей
		if (key_is_entered_) {
			throw std::logic_error("\"EndDict\": calling after Key"s);
		}

		// Удаление узла из стека заполнения
		node_stack_.pop_back();

		return { *this };

	}

	// Завершает определение сложного значения-массива. Последним незавершённым
	// вызовом Start* должен быть StartArray.
	AfterNoLim Builder::EndArray() {
		// Что объект не только что создан
		if (ObjectJustCreated()) {
			throw std::logic_error("\"EndArray\": called for empty object"s);
		}

		// Проверка, что корневой узел список, если стек пуст, или последний узел в стеке - список
		if ((node_stack_.empty() && !root_.IsArray()) || (!node_stack_.empty() && !node_stack_.back()->IsArray())) {
			throw std::logic_error("\"EndArray\": called for not dict"s);
		}

		// Удаление узла из стека заполнения
		node_stack_.pop_back();

		// Заглушка
		return { *this };
	}

	// Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами
	// методов. К этому моменту для каждого Start* должен быть вызван соответствующий
	// End*. При этом сам объект должен быть определён, то есть вызов
	// json::Builder{}.Build() недопустим.
	Node Builder::Build() {
		if (!ObjectIsFinished()) {
			throw std::logic_error("\"Build\": called for not finished object"s);
		}

		// Возврат корневого узла
		return root_;
	}


	// --------- Блок логических проверок состояния ---------

	// Проверка, что объект только что создан
	bool Builder::ObjectJustCreated() {
		return root_.IsNull() && node_stack_.empty();
	}

	// Проверка готовности объекта
	bool Builder::ObjectIsFinished() {
		return !root_.IsNull() && node_stack_.empty();
	}

	// Проверка, что вызов после конструктора или после Key или после предыдущего элемента массива.
	bool Builder::CheckCorrectCall() {
		// Проверка вызова после метода Key
		bool after_key = !node_stack_.empty() && node_stack_.back()->IsDict() && key_is_entered_; //!key_buffer_.empty();

		// Проверка вызова в Array
		bool in_array = !node_stack_.empty() && node_stack_.back()->IsArray();

		return ObjectJustCreated() || after_key || in_array;
	}


	// Создание узла Array/Dict
	bool Builder::StartList(Node&& node) {
		// Создание node сразу после конструктора
		if (ObjectJustCreated()) {
			root_ = node;

			// Добаление указателя на корневой узел, для блокировки Build.
			// Будет удален при вызове End*. Это значит, что node_stack_ всегда содержит
			// значение, если конструирование не завершено
			node_stack_.push_back(&root_);

			return true;
		}

		// Привязка к последнему узлу в node_stack_
		auto& last_node = *node_stack_.back();

		if (last_node.IsArray()) {
			// Возврат массива из узла
			Array buffer_array = last_node.AsArray();

			// Внесение node в буфферный массив
			buffer_array.push_back(std::move(node));

			// Получение указателя на узел со словарем
			Node* array_ptr = &buffer_array.back();

			// Перенос узла в стек
			node_stack_.push_back(std::move(array_ptr));

			// Присвоение нового узла для root_
			last_node = std::move(Node(std::move(buffer_array)));

			return true;
		}
		else if (last_node.IsDict()) {
			Dict buffer_dict = last_node.AsDict();

			buffer_dict[key_buffer_] = std::move(node);

			Node* array_ptr = &buffer_dict[key_buffer_];
			node_stack_.push_back(std::move(array_ptr));

			key_is_entered_ = false;

			last_node = std::move(Node(std::move(buffer_dict)));

			return true;
		}
		else {
			throw std::logic_error("\"StartList\": incorrect call for not empty node_stack_"s);
		}

		return false;
	}




	// --------- Класс AfterNoLim ----------

	AfterKey AfterNoLim::Key(std::string key) {
		return builder_.Key(key);
	}

	AfterNoLim AfterNoLim::Value(Node::Value value) {
		return builder_.Value(value);
	}

	AfterStartDict AfterNoLim::StartDict() {
		return builder_.StartDict();
	}

	AfterStartArray AfterNoLim::StartArray() {
		return builder_.StartArray();
	}

	AfterNoLim AfterNoLim::EndDict() {
		return builder_.EndDict();
	}

	AfterNoLim AfterNoLim::EndArray() {
		return builder_.EndArray();
	}

	Node AfterNoLim::Build() {
		return builder_.Build();
	}


	// Вспомогательные методы вспомогательных классов для обработки Value

	AfterKeyValue AfterKey::Value(Node::Value value) {
		return { builder_.Value(value) };
	}

	AfterArrayValue AfterStartArray::Value(Node::Value value) {
		return { builder_.Value(value) };
	}

	AfterArrayValue AfterArrayValue::Value(Node::Value value) {
		return { builder_.Value(value) };
	}




} // End of json