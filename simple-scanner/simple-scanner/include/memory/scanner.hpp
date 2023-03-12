#ifndef SCANNER_HPP
#define SCANNER_HPP

namespace scanner
{
	class c_memory
	{
	private:
		MEMORY_BASIC_INFORMATION _mbi {};
		auto read_memory( const HANDLE, const std::string ) -> bool;
		auto get_process_id_by_name( const std::string ) -> int;

	public:
		auto initialize( const std::pair<std::string, std::string> ) -> void;

		c_memory( ) = default;
		~c_memory( ) = default;

	};
}

#endif // !SCANNER_HPP