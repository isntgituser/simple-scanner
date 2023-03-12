#include "pch.hpp"

#include "menu\menu.hpp"
#include "memory\scanner.hpp"

namespace menu
{
	LRESULT WINAPI c_window::wnd_proc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
	{
		if ( ImGui_ImplWin32_WndProcHandler( hwnd, message, wparam, lparam ) )
			return true;

		switch ( message )
		{
			case WM_QUIT:
				PostQuitMessage( 0 );
				break;

			case WM_NCHITTEST:
				RECT rect {};
				GetWindowRect( hwnd, &rect );

				POINT current_pos {};
				GetCursorPos( &current_pos );

				auto hit = DefWindowProcA( hwnd, message, wparam, lparam );
				if ( hit == HTCLIENT && ( current_pos.y < ( rect.top + 15 ) ) )
					return HTCAPTION;

				break;
		}

		return DefWindowProcA( hwnd, message, wparam, lparam );
	}

	bool c_window::create( const std::string w_name, const ImVec2 w_size, context_t &w_context, const std::function<void( context_t & )> w_styles )
	{
		try
		{
			const auto init_centered = [ ]( HWND hwnd )
			{
				RECT rc;
				GetWindowRect( hwnd, &rc );

				auto size = std::make_pair( GetSystemMetrics( SM_CXSCREEN ) - rc.right, GetSystemMetrics( SM_CYSCREEN ) - rc.bottom );

				SetWindowPos( hwnd, nullptr, size.first / 2, size.second / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
			};

			const auto create_device = [ & ]( )
			{
				w_context.d3d = Direct3DCreate9( D3D_SDK_VERSION );
				if ( w_context.d3d == nullptr )
					return false;

				std::memset( &w_context.params, 0, sizeof( w_context.params ) );

				w_context.params.Windowed = true;
				w_context.params.SwapEffect = D3DSWAPEFFECT_DISCARD;
				w_context.params.BackBufferFormat = D3DFMT_UNKNOWN;
				w_context.params.EnableAutoDepthStencil = true;
				w_context.params.AutoDepthStencilFormat = D3DFMT_D16;
				w_context.params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

				if ( w_context.d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, w_context.hwnd, 0x00000040L, &w_context.params, &w_context.device ) < 0 )
					return false;

				return true;
			};

			w_context.wc =
			{
				sizeof( WNDCLASSEXA ),
				CS_CLASSDC,
				( WNDPROC ) wnd_proc,
				0L,
				0L,
				GetModuleHandleA( nullptr ),
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				w_name.data( ),
				nullptr
			};

			RegisterClassExA( &w_context.wc );

			w_context.hwnd = CreateWindowExA( 0, w_context.wc.lpszClassName, w_context.wc.lpszClassName,
											  WS_POPUP, 100, 100, w_size.x, w_size.y,
											  nullptr, nullptr, w_context.wc.hInstance, nullptr );

			if ( w_context.hwnd == nullptr )
				throw std::exception( "error while creating the window" );

			if ( !create_device( ) )
				throw std::exception( "cannot create device!" );

			init_centered( w_context.hwnd );

			ShowWindow( w_context.hwnd, SW_SHOWDEFAULT );
			UpdateWindow( w_context.hwnd );

			ImGui::CreateContext( );

			w_styles( w_context );

			ImGui_ImplWin32_Init( w_context.hwnd );
			ImGui_ImplDX9_Init( w_context.device );

			return true;
		}
		catch ( std::exception &e )
		{
			cleanup_device( w_context );

			MessageBoxA( nullptr, e.what( ), nullptr, MB_OK | MB_ICONERROR );
			return false;
		}

		return false;
	}

	void c_window::render( context_t &w_context, const ImVec2 w_size, const std::function<void( context_t &, const ImVec2 )> menu )
	{
		MSG msg;
		std::memset( &msg, 0, sizeof( msg ) );

		while ( msg.message != WM_QUIT )
		{
			if ( PeekMessageA( &msg, nullptr, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessageA( &msg );
				continue;
			}

			ImGui_ImplDX9_NewFrame( );
			ImGui_ImplWin32_NewFrame( );
			ImGui::NewFrame( );

			menu( w_context, w_size );

			ImGui::EndFrame( );

			w_context.device->Clear( 0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );

			if ( !w_context.context_state )
			{
				msg.message = WM_QUIT;
			}

			if ( w_context.device->BeginScene( ) >= 0 )
			{
				ImGui::Render( );
				ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );
				w_context.device->EndScene( );
			}

			if ( w_context.device->Present( nullptr, nullptr, nullptr, nullptr ) == D3DERR_DEVICELOST && w_context.device->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
			{
				ImGui_ImplDX9_InvalidateDeviceObjects( );
				w_context.device->Reset( &w_context.params );
				ImGui_ImplDX9_CreateDeviceObjects( );
			}
		}

		ImGui_ImplDX9_Shutdown( );
		ImGui_ImplWin32_Shutdown( );
		ImGui::DestroyContext( );

		cleanup_device( w_context );

		DestroyWindow( w_context.hwnd );
		UnregisterClassA( w_context.wc.lpszClassName, w_context.wc.hInstance );
	}

	void c_render::menu_style( context_t &w_context )
	{
		auto &io = ImGui::GetIO( );
		auto &style = ImGui::GetStyle( );

		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		open_sans_bd = io.Fonts->AddFontFromMemoryCompressedBase85TTF( fonts::open_sans_bd.data( ), 18 );
		open_sans_sm = io.Fonts->AddFontFromMemoryCompressedBase85TTF( fonts::open_sans_bd.data( ), 15 );
		open_sans = io.Fonts->AddFontFromMemoryCompressedBase85TTF( fonts::open_sans.data( ), 17 );

		style.Colors[ ImGuiCol_WindowBg ] = ImColor( 18, 18, 18 );
		style.Colors[ ImGuiCol_Border ] = ImColor( 17, 17, 17 );

		style.Colors[ ImGuiCol_FrameBg ] = ImColor( 31, 31, 31 );
		style.Colors[ ImGuiCol_FrameBgActive ] = ImColor( 31, 31, 31 );
		style.Colors[ ImGuiCol_FrameBgHovered ] = ImColor( 31, 31, 31 );

		style.Colors[ ImGuiCol_Button ] = ImColor( 74, 19, 243 );
		style.Colors[ ImGuiCol_ButtonActive ] = ImColor( 74, 19, 243 );
		style.Colors[ ImGuiCol_ButtonHovered ] = ImColor( 74, 19, 243 );

		style.Colors[ ImGuiCol_TextSelectedBg ] = ImColor( 74, 19, 243 );

		style.Colors[ ImGuiCol_Header ] = ImColor( 74, 19, 243 );
		style.Colors[ ImGuiCol_HeaderActive ] = ImColor( 74, 19, 243 );
		style.Colors[ ImGuiCol_HeaderHovered ] = ImColor( 74, 19, 243 );

		style.Colors[ ImGuiCol_Separator ] = ImColor( 51, 51, 51 );

		style.WindowPadding = { 0,0 };
		style.WindowBorderSize = 0;
		style.ChildRounding = 5;
		style.FrameRounding = 2;
		style.ScrollbarSize = 2;
	}

	void c_render::menu_panel( context_t &w_context, const ImVec2 w_size )
	{
		ImGui::SetNextWindowPos( { 0, 0 }, ImGuiCond_::ImGuiCond_Always );
		ImGui::SetNextWindowSize( w_size, ImGuiCond_::ImGuiCond_Always );

		const auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
		ImGui::Begin( "###main_painel", nullptr, flags );
		{
			const auto draw = ImGui::GetWindowDrawList( );

			draw->AddLine( { 10, 12 }, { 10, 28 }, ImColor( 74, 19, 243, 255 ), 2.0f );

			ImGui::SetCursorPos( { 17, 10 } );
			ImGui::Text( "Simple Scanner" );

			ImGui::SetCursorPosY( { 43 } );
			ImGui::Separator( );

			static std::string search_str {};

			static std::array<const char *, 3> proc_list { "Explorer.exe", "SearchIndexer.exe", "lsass.exe" };
			static int num { 0 };

			ImGui::PushFont( open_sans );
			{
				ImGui::PushItemWidth( 180 );
				{
					draw->AddLine( { 10, 70 }, { 10, 93 }, ImColor( 74, 19, 243, 255 ), 2.0f );

					ImGui::SetCursorPos( { 17, 70 } );
					ImGui::InputTextWithHint( "###string_label", "write your string here...", &search_str );

					draw->AddLine( { 10, 100 }, { 10, 123 }, ImColor( 74, 19, 243, 255 ), 2.0f );

					ImGui::SetCursorPos( { 17, 100 } );
					ImGui::Combo( "###process_combo", &num, proc_list.data( ), proc_list.size( ) );
				}
				ImGui::PopItemWidth( );
			}
			ImGui::PopFont( );

			ImGui::SetCursorPos( { 9, 130 } );
			if ( ImGui::Button( "start scan", { 90, 25 } ) )
			{
				const auto scanner = std::make_unique<scanner::c_memory>( );
				scanner->initialize( { proc_list[ num ], search_str } );
			}

			ImGui::SetCursorPos( { 105, 130 } );
			if ( ImGui::Button( "exit", { 92, 25 } ) )
			{
				w_context.context_state = false;
			}

			ImGui::PushFont( open_sans_sm );
			{
				ImGui::SetCursorPos( { 225, 180 } );
				ImGui::TextColored( ImColor( 155, 155, 155 ), "Github" );
				if ( ImGui::IsItemHovered( 0 ) )
					ImGui::SetTooltip( "click me!" );
				if ( ImGui::IsItemClicked( 0 ) )
					ShellExecuteA( 0, 0, "https://github.com/isntgituser", 0, 0, SW_SHOW );
			}
			ImGui::PopFont( );
		}
		ImGui::End( );
	}
}