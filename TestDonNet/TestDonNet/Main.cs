using System;
using System.Runtime.InteropServices;

namespace TestDonNet
{
	class MainClass
	{
		[DllImport ("libc")]
		private static extern int getpid ();

		[DllImport ("libnative")]
		private static extern long 	rt_timer_read();

		[DllImport ("libxenomai")]
		private static extern int rt_print_init(int buffer_size, string name);

		[DllImport ("libnative")]
		private static extern long rt_timer_ticks2ns(long ns);

		[DllImport ("libdotnet-sdk")] //, EntryPoint="_Z3addv", CallingConvention=CallingConvention.Cdecl
		private static extern int add(int a);


		public static void Main (string[] args)
		{
			rt_print_init(10, "md");

			Console.WriteLine ("rt_timer_ticks2ns() = {0}", rt_timer_ticks2ns(rt_timer_read()));
			//add();
			Console.WriteLine ("add = {0}", add(14));
		}
	}
}
