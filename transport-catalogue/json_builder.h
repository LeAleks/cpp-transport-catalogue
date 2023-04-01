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

class Builder{
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
	// �������������� ������ (������ root_ � Document)
	Node root_;

	// ���� ���������� �� �� ������� JSON, ������� ��� �� ���������: �� ����
	// ������� ����������� �������� � ������� ��� ���������. �� ������� ������������
	// � ������ �������� ����� ������ End-�������.
	std::vector<Node*> node_stack_;

	// ������ �������� ����� ��� ��������� ������� � ��� ������
	std::string key_buffer_;
	bool key_is_entered_ = false;

	// ��������, ��� ������ ������ ��� ������
	bool ObjectJustCreated();

	// �������� ���������� �������
	bool ObjectIsFinished();

	// ��������, ��� ����� ����� ������������ ��� ����� Key ��� ����� ����������� �������� �������.
	bool CheckCorrectCall();

	// �������� ���� ��� Array/Dict
	bool StartList(Node&& node);

};


// --- ������ ��� ��������� ������ ���������� �� ����� ���������� ---

// 0. ��� ����������� (������������ �����)
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

// 1. ��������������� ����� Key ������ Value, StartDict ��� StartArray.
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

	// ������� � ������� 2
	AfterKeyValue Value(Node::Value value);
};

// 2. ����� ������ Value, �������������� �� ������� Key, ������ Key ��� EndDict
class AfterKeyValue : public AfterNoLim {
public:
	// ����������� �� AfterNoLim, �.�. ������� Value ���������� ������ ���� �����
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

// 3. �� ������� StartDict ������� Key ��� EndDict
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

// 4. �� ������� StartArray ������� Value, StartDict, StartArray ��� EndArray
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

	// ������� � ������� 5
	AfterArrayValue Value(Node::Value value);
};

// 5. ����� ������ StartArray � ����� Value ������� Value, StartDict, StartArray ��� EndArray
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

	// ����������� ��������� ������� 5
	AfterArrayValue Value(Node::Value value);
};




} // End of json