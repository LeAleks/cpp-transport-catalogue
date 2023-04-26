#pragma once

#include "json.h"

#include <vector>
#include <string>


namespace json {

	class AfterNoLim;
	class AfterKey;
	class AfterKeyValue;
	class AfterStartDict;
	class AfterStartArray;
	class AfterArrayValue;

	class Builder {
	public:
		Builder()
			: root_(nullptr) {}

		AfterKey Key(std::string key);
		AfterNoLim Value(Node::Value value);
		AfterStartDict StartDict();
		AfterStartArray StartArray();
		AfterNoLim EndDict();
		AfterNoLim EndArray();
		Node Build();


	private:
		// Конструируемый объект (аналог root_ у Document)
		Node root_;

		// стек указателей на те вершины JSON, которые ещё не построены: то есть
		// текущее описываемое значение и цепочка его родителей. Он поможет возвращаться
		// в нужный контекст после вызова End-методов.
		std::vector<Node*> node_stack_;

		// Буффер хранения ключа при заполении словаря и его статус
		std::string key_buffer_;
		bool key_is_entered_ = false;

		// Проверка, что объект только что создан
		bool ObjectJustCreated();

		// Проверка готовности объекта
		bool ObjectIsFinished();

		// Проверка, что вызов после конструктора или после Key или после предыдущего элемента массива.
		bool CheckCorrectCall();

		// Создание узла для Array/Dict
		bool StartList(Node&& node);

	};


	// --- Классы для обработки ошибок синтаксиса на этапе компиляции ---

	// 0. Без ограничений (Родительский класс)
	class AfterNoLim {
	public:
		AfterNoLim(Builder& builder)
			: builder_(builder) {}

		AfterKey Key(std::string key);

		AfterNoLim Value(Node::Value value);

		AfterStartDict StartDict();

		AfterStartArray StartArray();

		AfterNoLim EndDict();

		AfterNoLim EndArray();

		Node Build();

		Builder& builder_;
	};

	// 1. Непосредственно после Key вызван Value, StartDict или StartArray.
	class AfterKey : public AfterNoLim {
	public:
		AfterKey(Builder& builder)
			: AfterNoLim(builder) {}

		AfterKey Key(std::string key) = delete;
		//AfterNoLim Value(Node::Value value) = delete;
		//AfterStartDict StartDict() = delete;
		//AfterStartArray StartArray() = delete;
		AfterNoLim EndDict() = delete;
		AfterNoLim EndArray() = delete;
		Node Build() = delete;

		// Переход к правилу 2
		AfterKeyValue Value(Node::Value value);
	};

	// 2. После вызова Value, последовавшего за вызовом Key, вызван Key или EndDict
	class AfterKeyValue : public AfterNoLim {
	public:
		// Конструктор от AfterNoLim, т.к. базовый Value возвращает именно этот класс
		AfterKeyValue(AfterNoLim no_lim)
			: AfterNoLim(no_lim.builder_) {}

		//AfterKey Key(std::string key) = delete;
		AfterNoLim Value(Node::Value value) = delete;
		AfterStartDict StartDict() = delete;
		AfterStartArray StartArray() = delete;
		//AfterNoLim EndDict() = delete;
		AfterNoLim EndArray() = delete;
		Node Build() = delete;
	};

	// 3. За вызовом StartDict следует Key или EndDict
	class AfterStartDict : public AfterNoLim {
	public:
		AfterStartDict(Builder& builder)
			: AfterNoLim(builder) {}

		//AfterKey Key(std::string key) = delete;
		AfterNoLim Value(Node::Value value) = delete;
		AfterStartDict StartDict() = delete;
		AfterStartArray StartArray() = delete;
		//AfterNoLim EndDict() = delete;
		AfterNoLim EndArray() = delete;
		Node Build() = delete;
	};

	// 4. За вызовом StartArray следует Value, StartDict, StartArray или EndArray
	class AfterStartArray : public AfterNoLim {
	public:
		AfterStartArray(Builder& builder)
			: AfterNoLim(builder) {}

		AfterKey Key(std::string key) = delete;
		//AfterNoLim Value(Node::Value value) = delete;
		//AfterStartDict StartDict() = delete;
		//AfterStartArray StartArray() = delete;
		AfterNoLim EndDict() = delete;
		//AfterNoLim EndArray() = delete;
		Node Build() = delete;

		// Переход к правилу 5
		AfterArrayValue Value(Node::Value value);
	};

	// 5. После вызова StartArray и серии Value следует Value, StartDict, StartArray или EndArray
	class AfterArrayValue : public AfterNoLim {
	public:
		AfterArrayValue(AfterNoLim no_lim)
			: AfterNoLim(no_lim.builder_) {}

		AfterKey Key(std::string key) = delete;
		//AfterNoLim Value(Node::Value value) = delete;
		//AfterStartDict StartDict() = delete;
		//AfterStartArray StartArray() = delete;
		AfterNoLim EndDict() = delete;
		//AfterNoLim EndArray() = delete;
		Node Build() = delete;

		// Продолжение выполения правила 5
		AfterArrayValue Value(Node::Value value);
	};




} // End of json