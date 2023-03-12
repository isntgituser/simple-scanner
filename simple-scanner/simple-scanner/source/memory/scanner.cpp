#include "pch.hpp"
#include "memory\scanner.hpp"

namespace scanner
{
    auto c_memory::read_memory( const HANDLE proc, const std::string str ) -> bool
    {
        std::vector<std::string> buffer {};
        for ( size_t address = 0; VirtualQueryEx( proc, reinterpret_cast< LPVOID >( address ), &_mbi, sizeof( _mbi ) ); address += _mbi.RegionSize )
        {
            if ( !( _mbi.State == MEM_COMMIT && _mbi.Protect != PAGE_NOACCESS && _mbi.Protect != PAGE_GUARD ) )
                continue;

            buffer.resize( buffer.size( ) + 1 );
            buffer[ buffer.size( ) - 1 ].resize( _mbi.RegionSize, 0 );

            if ( !ReadProcessMemory( proc, reinterpret_cast< LPVOID >( address ), &buffer[ buffer.size( ) - 1 ][ 0 ], _mbi.RegionSize, nullptr ) )
                continue;
        }

        for ( const auto &buff : buffer )
        {
            if ( buff.find( str ) != std::string::npos )
                return true;
        }

        return false;
    }

    auto c_memory::get_process_id_by_name( const std::string name ) -> int
    {
        const auto to_lower = [ ]( std::string str )
        {
            std::transform( str.begin( ), str.end( ), str.begin( ), static_cast< int( * )( int ) >( ::tolower ) );
            return str;
        };

        if ( name.empty( ) )
            return false;

        auto str_fl = name;
        if ( str_fl.find_last_of( "." ) != std::string::npos )
            str_fl.erase( str_fl.find_last_of( "." ), std::string::npos );

        str_fl += ".exe";

        const auto handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
        PROCESSENTRY32 entry; entry.dwSize = sizeof( entry );

        if ( !Process32First( handle, &entry ) )
            return false;

        do
        {
            if ( to_lower( entry.szExeFile ).compare( to_lower( str_fl ) ) == 0 )
            {
                CloseHandle( handle );
                return entry.th32ProcessID;
            }
        } while ( Process32Next( handle, &entry ) );

        CloseHandle( handle );
        return 0;
    }

    auto c_memory::initialize( const std::pair<std::string, std::string> info ) -> void
    {
        const auto process_id = this->get_process_id_by_name( info.first );

        if ( this->read_memory( OpenProcess( PROCESS_ALL_ACCESS, false, process_id ), info.second ) )
            MessageBoxA( nullptr, "String Found!", nullptr, MB_OK | MB_ICONINFORMATION );
        else
            MessageBoxA( nullptr, "Not String Found!", nullptr, MB_OK | MB_ICONINFORMATION );
    }
}