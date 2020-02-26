import java.util.*;
import java.lang.*;
import java.io.*;

class absDataCollect
{
	void do_(){}
	void dataCollect(){}
	public static void main (String[] args) throws java.lang.Exception{
        System.out.println("Hello! World!");
        absDataCollect test1=new BData();
        absDataCollect test2=new CData();
        dataDoSomethig dds=new dataDoSomethig();
        dds.dataDoSomethig(test1);
        dds.dataDoSomethig(test2);
    }
	static class BData extends absDataCollect{
		void dataCollect()
		{
			
		}
		void do_()
		{
			 System.out.println("B data call service\n");
		}
	}
	static class CData extends absDataCollect{
		void dataCollect()
		{
			
		}
		void do_()
		{
			System.out.println("C data call service\n");
		}
	}
	static class dataDoSomethig{
		void dataDoSomethig(absDataCollect data)
		{
			data.do_();
		}
	}
}

