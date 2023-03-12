#ifndef MENU_HPP
#define MENU_HPP

extern auto ImGui_ImplWin32_WndProcHandler( HWND, UINT, WPARAM, LPARAM ) -> IMGUI_API LRESULT;

namespace menu
{
	struct context_t
	{
		HWND					hwnd {};
		LPDIRECT3D9				d3d {};
		LPDIRECT3DDEVICE9		device {};
		D3DPRESENT_PARAMETERS	params {};

		WNDCLASSEXA wc {};
		bool context_state { true };
	};

	class c_window
	{
	private:
		static LRESULT WINAPI wnd_proc( HWND, UINT, WPARAM, LPARAM );
		void cleanup_device( context_t &w_context )
		{
			if ( w_context.d3d )
			{
				w_context.d3d->Release( );
				w_context.d3d = nullptr;
			}

			if ( w_context.device )
			{
				w_context.device->Release( );
				w_context.device = nullptr;
			}
		}

	public:
		bool create( const std::string, const ImVec2, context_t &, const std::function<void( context_t & )> );
		void render( context_t &, const ImVec2, const std::function<void( context_t &, const ImVec2 )> );

		c_window( ) = default;
		~c_window( ) = default;
	};

	class c_render
	{
	private:
		inline static ImFont *open_sans, *open_sans_bd, *open_sans_sm;

	public:
		static void menu_style( context_t & );
		static void menu_panel( context_t &, const ImVec2 );

		c_render( ) = default;
		~c_render( ) = default;

	};
}

#endif // ~ !MENU_HPP