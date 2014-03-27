using System;
using System.Runtime.InteropServices;

namespace TestDonNet
{
	class MainClass
	{
		delegate int MyCallback1 (int a, int b);
 
		[DllImport ("libdotnet-sdk")]
		extern static void RegisterCallback (MyCallback1 callback1);

		[DllImport ("libdotnet-sdk")]
		extern static int Start();
 
		[DllImport ("libdotnet-sdk")]
		extern static int Stop();


		static int Add (int a, int b) 
		{
			Console.WriteLine ("A = {0} B = {1}", a,b);
			return a + b; 
		}


		public static void Main (string[] args)
		{
			RegisterCallback(Add);

			Console.WriteLine ("Start = {0}", Start());

			Console.ReadLine();

			Console.WriteLine ("Stop = {0}", Stop());

//scp -P 1234 /home/vrubel/Projects/LinuxDrone/build.Debug/libraries/dotnet-sdk/libdotnet-sdk.so root@voha-bbb.linuxdrone.org:/root/vrubel/libdotnet-sdk.so
//scp -P 1234 /home/vrubel/Projects/TestDonNet/TestDonNet/bin/Debug/TestDonNet.exe root@voha-bbb.linuxdrone.org:/root/vrubel/TestDonNet.exe

		}
	}
}
