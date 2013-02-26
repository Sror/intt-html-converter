#pragma once

#include <iostream>

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace html { namespace impl {

	enum NodeType
	{
		Unknown,
		Root,
		Text,
		Tag,
		UnicodeChar,
		TagName,
		TagValue,
	};

	struct AstData
	{
		NodeType type;
		std::string value;
	};

	class AstNode
	{
	public:
		AstNode() { data_.type = Unknown; }
		explicit AstNode(const AstData& val) : data_(val) {}

		~AstNode() {}

		AstNode(const AstNode& rhs) { *this = rhs; }
		explicit AstNode(const std::string& rhs) { *this = rhs; }

		AstNode& operator=(const AstNode& rhs) {
			if (&rhs == this) {
				return *this;
			}
			data_ = rhs.data_;
			children_.clear();
			std::copy(rhs.children_.begin(), rhs.children_.end(), std::back_inserter(children_));

			return *this;
		}

		AstNode& operator=(const std::string& rhs) {
			data_.value = rhs;
			data_.type = Text;
			children_.clear();
			return *this;
		}

		AstData data() const { return data_; }
		AstData& data() { return data_; }

		static void print_preorder(const AstNode* const root, std::ostream& os, size_t indent = 0) {
			if (!root) {
				return;
			}
			for (size_t i = 0; i < indent; ++i) {
				os << ' ';
			}
			switch (root->data().type) {
			case Unknown:
				os << "Unknown: ";
				break;
			case Root:
				os << "Document: ";
				break;
			case Text:
				os << "Text: ";
				break;
			case Tag:
				os << "Tag: ";
				break;
			case TagName:
				os << "TagName: ";
				break;
			case TagValue:
				os << "TagValue: ";
				break;
			case UnicodeChar:
				os << "UnicodeChar: ";
				break;
			default:
				os << "Undefined: ";
				break;
			}

			os << root->data().value << "\n";

			for (children_type::const_iterator i = root->children_.begin(); i != root->children_.end(); ++i) {
				print_preorder(i->get(), os, indent + 4);
			}
		}

		void add_child(boost::shared_ptr<AstNode> node) {
			children_.push_back(node);
		}

		boost::shared_ptr<AstNode> child(size_t i) const {
			if (i < children_.size()) {
				return children_[i];
			}
			return boost::shared_ptr<AstNode>();
		}

	private:
		typedef std::vector<boost::shared_ptr<AstNode> > children_type;

		AstData data_;
		std::vector<boost::shared_ptr<AstNode> > children_;
	};

}}
