#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
extern "C"
{
extern void   Abc_Start();
extern void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
extern void * Abc_FrameGetGlobalFrame();
extern int    Cmd_CommandExecute( void * pAbc, char * sCommand );
}

int call_abc()
{

    // variables
    void * pAbc;
    FILE *fp;
    char Command[100];
    
     //////////////////////////////////////////////////////////////////////////
     // start the ABC framework
     Abc_Start();
     pAbc = Abc_FrameGetGlobalFrame();

    //////////////////////////////////////////////////////////////////////////
    // open the name file
/*    if((fp = fopen("bench.log","r"))==NULL)
    {
    	printf("Cannot open file bench.log!");
        return 1;
    }*/
	    
	//////////////////////////////////////////////////////////////////////////
    // read_bench
    sprintf( Command, "read_bench bench.log" );
	if ( Cmd_CommandExecute( pAbc, Command ) )
	{
	    fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
	    return 2;
	}
	
	//////////////////////////////////////////////////////////////////////////
    // strash
    sprintf( Command, "strash" );
	if ( Cmd_CommandExecute( pAbc, Command ) )
	{
	    fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
	    return 3;
	}
	
	//////////////////////////////////////////////////////////////////////////
    // write_cnf
    sprintf( Command, "write_cnf bench.cnf" );
	if ( Cmd_CommandExecute( pAbc, Command ) )
	{
	    fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
	    return 4;
	}
	
    //////////////////////////////////////////////////////////////////////////
    // stop the ABC framework
    Abc_Stop();
    return 0;   
}

