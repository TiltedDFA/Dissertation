#include "l2sb.hpp"
#include <string>

void L2SB_BONN_TEST(int O, int S, int H);
void L2SB_MITBIH_TEST(int Term, char * BandConfig);

float CRU =0;
float CRT =0;


int n=12;

int main(void)
{		
		
	printf("\n");
	printf("#####################################\n");
	printf("####### L2SB Simulation TOOLS #######\n");
	printf("####### (c)2024 Univ. of York #######\n");
	printf("#####################################\n");
	printf("\n");


	/////////////////////////////////////////
	// initialise memory & default Settings

	    if(Init()==0){ return 0;}

	    SetBitWidth(11);	    	
	    SetFileStep(1);
	    SetMaxBands(-1);
	    SetTruncatedHeaders(1);
	    SetUniformHeaders(1);
	    SetDataLimit(-1);
	    SetBipolarData();
	    SetShowQuantisation(0);
	    SetResultsOutput(0);
      	   
    /////////////////////////////////////////
    // test case
        
       float CR = 0;
       
        if(1) // MITBIH ECG
      	{
      		SetFileStep(1);			// test every nth file, 1 = all files //
	    	SetMaxBands(-1);		// -1 do not limit the number of bands, n = max bands //
	    	// SetDataLimit(100000);		// -1 read all file, otherwise read n values //
	    	SetDataLimit(-1);		// -1 read all file, otherwise read n values //
	    	SetUnipolarData();		// MIT data is unipolar //
	        SetTruncatedHeaders(1);
	    	SetUniformHeaders(1);

	        std::string output_file = "Output/test/Datatest_UHC1.csv";
        	std::string header_file = "Output/test/Headertest_UHC1.csv";
	    	SetOutputFile(output_file.data());
	    	SetHeaderFile(header_file.data());
	    	ReportConfig();
	    	
	    	// just run one configuration 
	    	// example is a 5-band example
        	std::string config_file = "4,4,2,1,1,0,-1";
       		L2SB_MITBIH_TEST(1, config_file.data());

       		printf("\n All Files Average CR(Iniform Header) %f CR(Truncated Header) %f\n",CRU,CRT);
      	  }

	/////////////////////////////////////////
	// exit cleanup

	   printf(" Program Terminating\n\n");
	   Cleanup();
}



void L2SB_MITBIH_TEST(int Term, char * BandConfig)
{
   CRU=0;
   CRT=0;
   
	///////////////////////////////////////////////////////
	// Get List of files and run  processing scenario   	
    // only process .csv files that are not empty
    // step through file list according to FileStep 

	   ConfigTestCase(TEST_CONFIG_MITBIH_ECG);
	   
	   		sprintf(FilePath,"Data/MITBIH");
	   
	   		GetFileList();
	   
	   		int c = 0;
	   		
	   		
	   	for(int i=0; i < FileCount; i=i+FileStep)
	     	{	   
	     	  if(strstr(FileList[i],".csv")!=NULL)
	     	  {
	            strcpy(FileName,FileList[i]);
		        ReadDataFile(FilePath,FileName,Term); 
		     	
		     	
		     	if(DataCount>0) 
		     	{ 
		     	   
		     	   test_L2SB(BandConfig); 
		     	   CRU += CRUB;
		     	   CRT += CRTB;
		     	}	
		    
		        c++;
		      }	      	
		      
		            
	     	}
	 	   
	CRU = CRU / (float)c;
	CRT = CRT / (float)c;	
}

