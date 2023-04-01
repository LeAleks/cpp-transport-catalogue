#include "json_builder.h"

#include <stdexcept>
#include <utility>

using namespace std::literals;


// ��������

// ��������� ������
//	� ������ ������������� ������� � �������� ��������� ��� ��� ������ ���������
//	���������� ���� std::logic_error � �������� ���������� �� ������.
//	��� ������ ����������� � ��������� ��������� :
//	  - ����� ������ Build ��� ��������� ����������� �������, �� ���� ����� �����
//		������������ ��� ��� ������������� �������� � ��������.
//    - ����� ������ ������, ����� Build, ��� ������� �������.
//	  - ����� ������ Key ������� ������� ��� ����� ����� ������� Key.
//	  - ����� Value, StartDict ��� StartArray ���-����, ����� ��� ����� ������������,
//		����� Key ��� ����� ����������� �������� �������.
//	  - ����� EndDict ��� EndArray � ��������� ������� ����������

namespace json {

// ��� ����������� ������� ����� ��������� �������� �����
// ��� ��������� ���� ����-��������. ��������� ����� ������ ����������� ������
// �������� ��������������� ����� ����� �������� � ������� ������ Value ���
// �������� ��� ����������� � ������� StartDict ��� StartArray.
AfterKey Builder::Key(std::string key){
	// �������� ������ ������� ������� ��� ����� ����� ������� Key
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


// ����� ��������, ��������������� ����� ��� ����������� �������, ���������
// ������� ������� ���, ���� ������� ����� ����� ������������ json::Builder,
// �� ���������� ��������������� JSON - �������.����� ��������� ��� �������
// ������ � ����� ��� ������ � ��� � ����� ������ ��� �������.
// ����� Node::Value � ��� ������� ��� �������� ������ Node, ������� variant
// � ������� ��������� ����� - ��������.�������� ��������� ����.
AfterNoLim Builder::Value(Node::Value value) {

	// ��������, ��� ������ �����: ����� root_ � ������ �������� ����
	if (ObjectIsFinished()) {
		throw std::logic_error("\"Value\": called for finished object."s);
	}

	if (!CheckCorrectCall()) {
		throw std::logic_error("\"Value\": called in wrong place."s);
	}

	// �������� value ����� ����� ������������
	if (ObjectJustCreated()) {
		root_ = std::move(Node(std::move(value)));

		return { *this };
	}

	// �������� � ���������� ���� � node_stack_
	auto& last_node = *node_stack_.back();

	if (last_node.IsArray()) {
		// ������� ������� �� ����
		Array buffer_array = last_node.AsArray();

		// �������� Value � ��������� ������
		buffer_array.push_back(std::move(Node(std::move(value))));

		// ���������� ������ ���� ��� root_
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

// �������� ����������� �������� �������� - �������.���������� � ��� �� ����������,
// ��� � Value. ��������� ������� ����������� ������ ���� Key ��� EndDict.
AfterStartDict Builder::StartDict(){
	// ��������, ��� ������ �����: ����� root_ � ������ �������� ����
	if (ObjectIsFinished()) {
		throw std::logic_error("\"StartDict\": called for finished object."s);
	}

	if (!CheckCorrectCall()) {
		throw std::logic_error("\"StartDict\": called in wrong place."s);
	}

	// ���� � ������ ��������
	Node empty_dict_n(Dict {});

	if (!StartList(std::move(empty_dict_n))) {
		throw std::logic_error("\"StartDict\": wasn't started"s);
	}

	return { *this };
}

// �������� ����������� �������� �������� - �������.���������� � ��� �� ����������,
// ��� � Value.��������� ������� ����������� ������ ���� EndArray ��� �����,
// �������� ����� �������� : Value, StartDict ��� StartArray.
AfterStartArray Builder::StartArray(){
	// ��������, ��� ������ �����: ����� root_ � ������ �������� ����
	if (ObjectIsFinished()) {
		throw std::logic_error("\"StartArray\": called for finished object."s);
	}

	if (!CheckCorrectCall()) {
		throw std::logic_error("\"StartArray\": called in wrong place."s);
	}

	// ���� � ������ �������
	Node empty_array_n(Array{});


	if (!StartList(std::move(empty_array_n))) {
		throw std::logic_error("\"StartArray\": wasn't started"s);
	}

	return { *this };
}

// ��������� ����������� �������� ��������-�������. ��������� �������������
// ������� Start* ������ ���� StartDict.
AfterNoLim Builder::EndDict(){
	// ��� ������ �� ������ ��� ������
	if (ObjectJustCreated()) {
		throw std::logic_error("\"EndDict\": called for empty object"s);
	}

	// ��������, ��� �������� ���� �������, ���� ���� ����, ��� ��������� ���� � ����� - �������
	if ((node_stack_.empty() && !root_.IsDict()) || (!node_stack_.empty() && !node_stack_.back()->IsDict())) {
		throw std::logic_error("\"EndDict\": called for not dict"s);
	}

	// ��������, ��� ��� ������������� ������
	if (key_is_entered_) {
		throw std::logic_error("\"EndDict\": calling after Key"s);
	}

	// �������� ���� �� ����� ����������
	node_stack_.pop_back();

	return { *this };

}

// ��������� ����������� �������� ��������-�������. ��������� �������������
// ������� Start* ������ ���� StartArray.
AfterNoLim Builder::EndArray(){
	// ��� ������ �� ������ ��� ������
	if (ObjectJustCreated()) {
		throw std::logic_error("\"EndArray\": called for empty object"s);
	}

	// ��������, ��� �������� ���� ������, ���� ���� ����, ��� ��������� ���� � ����� - ������
	if ((node_stack_.empty() && !root_.IsArray()) || (!node_stack_.empty() && !node_stack_.back()->IsArray())) {
		throw std::logic_error("\"EndArray\": called for not dict"s);
	}

	// �������� ���� �� ����� ����������
	node_stack_.pop_back();

	// ��������
	return { *this };
}

// ���������� ������ json::Node, ���������� JSON, ��������� ����������� ��������
// �������. � ����� ������� ��� ������� Start* ������ ���� ������ ���������������
// End*. ��� ���� ��� ������ ������ ���� ��������, �� ���� �����
// json::Builder{}.Build() ����������.
Node Builder::Build() {
	if (!ObjectIsFinished()) {
		throw std::logic_error("\"Build\": called for not finished object"s);
	}

	// ������� ��������� ����
	return root_;
}


// --------- ���� ���������� �������� ��������� ---------

// ��������, ��� ������ ������ ��� ������
bool Builder::ObjectJustCreated() {
	return root_.IsNull() && node_stack_.empty();
}

// �������� ���������� �������
bool Builder::ObjectIsFinished() {
	return !root_.IsNull() && node_stack_.empty();
}

// ��������, ��� ����� ����� ������������ ��� ����� Key ��� ����� ����������� �������� �������.
bool Builder::CheckCorrectCall() {
	// �������� ������ ����� ������ Key
	bool after_key = !node_stack_.empty() && node_stack_.back()->IsDict() && key_is_entered_; //!key_buffer_.empty();

	// �������� ������ � Array
	bool in_array = !node_stack_.empty() && node_stack_.back()->IsArray();

	return ObjectJustCreated() || after_key || in_array;
}


// �������� ���� Array/Dict
bool Builder::StartList(Node&& node) {
	// �������� node ����� ����� ������������
	if (ObjectJustCreated()) {
		root_ = node;

		// ��������� ��������� �� �������� ����, ��� ���������� Build.
		// ����� ������ ��� ������ End*. ��� ������, ��� node_stack_ ������ ��������
		// ��������, ���� ��������������� �� ���������
		node_stack_.push_back(&root_);

		return true;
	}

	// �������� � ���������� ���� � node_stack_
	auto& last_node = *node_stack_.back();

	if (last_node.IsArray()) {
		// ������� ������� �� ����
		Array buffer_array = last_node.AsArray();

		// �������� node � ��������� ������
		buffer_array.push_back(std::move(node));

		// ��������� ��������� �� ���� �� ��������
		Node* array_ptr = &buffer_array.back();

		// ������� ���� � ����
		node_stack_.push_back(std::move(array_ptr));

		// ���������� ������ ���� ��� root_
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




// --------- ����� AfterNoLim ----------

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


// ��������������� ������ ��������������� ������� ��� ��������� Value

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