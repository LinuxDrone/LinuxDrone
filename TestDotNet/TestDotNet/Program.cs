using System;
using System.Runtime.InteropServices;

namespace TestDotNet
{
	class MainClass
	{
		[DllImport ("libc-sdk")]
		private static extern int main ();

		[DllImport ("libc-sdk")]
		extern static void RegisterCallback (MyCallback1 callback1);

		[DllImport ("libnative")]
		extern static int rt_task_sleep (long delay);	


		delegate int MyCallback1 (int a, int b);

		public static void Main (string[] args)
		{
			RegisterCallback (Add);
			main ();
			Console.ReadLine ();
		}

		static int Add (int a, int b) 
		{ 
			//rt_task_sleep (100);
			return a + b; 
		}

		static int Sub (int a, int b) { return a - b; }
	}
}
