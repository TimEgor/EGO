#pragma once

#include <string>

namespace ego
{
	using FileNameID = uint32_t;
	inline constexpr FileNameID InvalidFileNameID = 0;

	class FileName final
	{
	public:
		using ResourceNameCharType = char;
		using ResourceNameContainerType = std::basic_string<ResourceNameCharType>;

		FileName() = default;
		FileName(const ResourceNameCharType* _newName);
		FileName(ResourceNameCharType _ch);
		FileName(const std::string& _str);
		FileName(const FileName& _newName);

		FileName& operator=(const ResourceNameCharType* _name);
		FileName& operator=(ResourceNameCharType _ch);
		FileName& operator=(const std::string& _str);
		FileName& operator=(const FileName& _name);
		FileName& operator+=(const ResourceNameCharType* _name);
		FileName& operator+=(ResourceNameCharType _ch);
		FileName& operator+=(const std::string& _str);
		FileName& operator+=(const FileName& _name);

		bool operator==(const ResourceNameCharType* _name) const;
		bool operator==(const std::string& _str) const;
		bool operator==(const FileName& _name) const;
		bool operator!=(const ResourceNameCharType* _name) const;
		bool operator!=(const std::string& _str) const;
		bool operator!=(const FileName& _name) const;

		operator bool() const { return !empty(); }

		const ResourceNameCharType& operator[](size_t _index) const;
		ResourceNameCharType& operator[](size_t _index);

		const ResourceNameCharType* c_str() const;
		size_t length() const;
		bool empty() const;

		void clear();
		void release();

		FileNameID hash() const;

	private:
		void assign(const ResourceNameCharType* _name);

		ResourceNameContainerType m_name;
	};

	FileName operator+(const FileName& _resName, const FileName::ResourceNameCharType* _name);
	FileName operator+(const FileName::ResourceNameCharType* _name, const FileName& _resName);
	FileName operator+(const FileName& _resName, FileName::ResourceNameCharType _ch);
	FileName operator+(FileName::ResourceNameCharType _ch, const FileName& _resName);
	FileName operator+(const FileName& _resName, const std::string& _str);
	FileName operator+(const std::string& _str, const FileName& _resName);
	FileName operator+(const FileName& _resName1, const FileName& _resName2);
}
