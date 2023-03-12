#include "pch.hpp"
#include "menu\menu.hpp"

auto wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow ) -> int __stdcall
{
	const auto window = std::make_unique<menu::c_window>( );
	const auto render = std::make_unique<menu::c_render>( );

	menu::context_t ctx {};
	if ( window->create( "simple scanner", { 270, 200 }, ctx, render->menu_style ) )
	{
		window->render( ctx, { 270, 200 }, render->menu_panel );
	}

	return EXIT_SUCCESS;
}