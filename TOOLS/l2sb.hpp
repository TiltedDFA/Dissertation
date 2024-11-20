#pragma once
#include <cstdio>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

inline int BitWidth         = 12;
inline int MaxBands         =  2;
inline long int DataLimit   = -1;
inline int TruncatedHeaders =  1;
inline int UniformHeaders   =  1;
inline int FileStep 	     =  1;
inline int Unipolar         =  0;
inline int ShowQuantisation =  0;
inline int ResultsFile      =  1;

inline int TEST_MODE=0;
#define TEST_COMPRESSION 1

#define MAX_DATA (4*1024*1024)

inline int *Data;

inline char FilePath[1024]="";
inline char FileName[1024]="";
inline char OutputFile[1024]="out.csv";
inline char HeaderFile[1024]="header.csv";

inline char FileList[2048][64]={"dummy"};
inline int  FileCount=0;
inline long int DataCount;

inline float DataRange = 16.0;
inline int Quantisation = 10;

void SetMaxBands(int x);
void SetTruncatedHeaders(int x);
void SetUniformHeaders(int x);
void SetFileStep(int x);
void SetDataLimit(long int x);
void SetBitWidth(int n);
void SetOutputFile(char* Str);
void SetHeaderFile(char* Str);
void SetUnipolarData();
void SetBipolarData();
void SetResultFile(int x);

void ReportConfig();

int Init();
void Cleanup();
void GetFileList();

void ReadDataFile(char * Path, char * FileName);
void ReadDataFile(char * Path, char * FileName, int Term);

int QuantiseData(float);
int QuantiseUnipolarData(float f);
int QuantiseBipolarData(float f);


void ConfigTestCase(int Selection);
float L2SBCompression(int b1,int b2, int b3, int b4, int h1, int h2,int h3,int h4);
float L2SB_MultiCompression(int * Band,int * Header);
void DecToBin(char * Bin, long int i, long int n);
void RPerms(int n);
void TruncatedBinary(int * Headers, int n);
void UniformBinary(int * Headers, int n);

float test_L2SB(char * BandConfig);

void L2SB();


#define TEST_CONFIG_DEFAULTS    0
#define TEST_CONFIG_BONN_EEG    1
#define TEST_CONFIG_MITBIH_ECG  2

inline int L2SBCompression_Debug = 0;


// #include <stdio.h>



/////////////////////////////////////////////


inline int Init()
{

	Data = static_cast<int*>(std::calloc(MAX_DATA, sizeof(int)));

	if(Data == nullptr)
	{
		printf(" Memory allocation problem \n");
		return(0);
	}
 
	printf(" Allocated memory %5.2f Mbytes \n", static_cast<double>(MAX_DATA)/(1024.0 * 1024.0));
	return(1);
}

inline void Cleanup()
{
	std::free(Data);
}

inline void GetFileList()
{
	FileCount=0;
	
	char Str[4096];
	sprintf(Str,"ls %s > filelist.txt",FilePath);
    system(Str);

    FILE* ip = fopen("filelist.txt", "r");

    if(ip == nullptr) { return;}
    
        while(1)
        {       
            fgets(Str,1020,ip);
            
            if(Str[strlen(Str)-1]<32){ Str[strlen(Str)-1]=0; }
            strcpy(FileList[FileCount],Str);
            
            if(feof(ip)==0) { FileCount++; } else {break; }
        }
    
    fclose(ip);
    
    // sprintf(Str,"rm filelist.txt", FilePath);
    sprintf(Str,"rm filelist.txt");
    system(Str);
}

inline void ReadDataFile(char * Path, char * FileName)
{
	ReadDataFile(Path, FileName, 0);
}


inline void ReadDataFile(char * Path, char * FileName, int Term)
{
	char Str[1024];

	int Max=0;
    int Val=0;
    
    sprintf(Str,"%s/%s",Path,FileName);
   
    
    FILE* ip = fopen(Str, "r");
    if(ip == nullptr) {printf("File Error %s", Str); return; }
    
    printf(" File import : %s\n",Str);
    printf("-----------------\n");
    DataCount=0;
    
    while(feof(ip)==0)
    {
        fgets(Str,1023,ip);
        
        // extract specific csv term
        int c=0;
        while(strstr(Str,",") != nullptr)
        	{
        		if(c>=Term){break;}
        		strcpy(Str,strstr(Str,",")+1);
           		c++;
        	}
        	
        float f = atof(Str);
        
        if(feof(ip)!=0) { break; }
        Val =  QuantiseData(f);
    
        Data[DataCount++] = Val;       
        if(f > Max) { Max=f; }      
        	
        if(DataLimit>0){  if(DataCount>DataLimit) { break; } }
    }
    
    DataCount--;
    
    printf(" File Imported [%s %s] , %ld data points quantised, data range <0-%d>\n",Path,FileName,DataCount,Max);
    printf("-----------------\n");
    
    fclose(ip);
}

inline int QuantiseData(float f)
{
	if(Unipolar==1)
	{
		return QuantiseUnipolarData(f);
	}
	else
	{
		return QuantiseBipolarData(f);		
	}
	
}

inline int QuantiseBipolarData(float f)
{
       float q =  f+DataRange;
       
       float Range = pow(2,Quantisation);
       float Rescale = Range/DataRange/2.0;
       
       q = q * Rescale;
       
       // printf("BIIPOLAR Read %5.2f Converted %5.2f. Rescale=%5.2f Range(%dbits)=%3.2f DataRange=%3.2f \n",f,q,Rescale, Quantisation,Range,DataRange); 

       return (int)q;
}

inline int QuantiseUnipolarData(float f)
{
	float q =  f;

	const float Range = pow(2,Quantisation);
	float Rescale = Range/DataRange;

	q = q * Rescale;

	if(ShowQuantisation)
    {
		printf("UNIPOLAR %5.2f Converted %5.2f. Rescale=%5.2f Range(%dbits)=%3.2f DataRange=%3.2f \n",f,q,Rescale, Quantisation,Range,DataRange);
	}

	return static_cast<int>(q);
}

inline void L2SB()
{
    int Q = Quantisation;
    int permcount=0;
    
    float CR[2]={0,0};   
    float CRmax[2] ={0,0};
    float CRmin[2] ={999,999};       
    
    FILE * op;
    FILE * hop;
    FILE * perms;
    
    if(ResultsFile==1)
    {
    	op = fopen(OutputFile,"a");
   		 if(op==nullptr) { printf("Output file error \n"); return; }
    
    	hop = fopen(HeaderFile,"r");
    	if(hop==nullptr) { hop = fopen(HeaderFile,"w"); }
    	else { hop = nullptr; }
    
    	fprintf(op,"\n%s/%s,",FilePath,FileName);
    }
    
    perms = fopen("perms.txt","r");
    if(perms==nullptr) { printf("permutation file error \n"); return; }
     
        
    
        
        if(1)
        {    char Line[2048];
        	 int Bands[32];
    		 int Headers[32];
    		 int b=0;
    		 
        	while(feof(perms)==0)
        		{
        			
        			fgets(Line,1020,perms);
        			b=0;
        		    	
        			while(strstr(Line,",")!=nullptr)
        				{
        					char * x = strstr(Line,",");
        					
        					x[0]=0;
        					Bands[b++]=atoi(Line);
        					strcpy(Line,&x[1]);
        				}
        				
        				Bands[b++]=atoi(Line);
        				Bands[b]=-1;
        				
        			
        		b++;
        		int Hcount = b;
        		//int Hwidth =  (log2(Hcount)+1);
        		
        		int Hwidth=1;
        		
        		if(Hcount>2){Hwidth=2;}
        		if(Hcount>4){Hwidth=3;}
        		if(Hcount>8){Hwidth=4;}
        		if(Hcount>16){Hwidth=5;}
        		if(Hcount>32){Hwidth=6;}
        		
        		if(Hwidth < 0){ Hwidth = 0;}
      
         		////////////////////////////////////////
        		// compression with truncated headers
        		  
        		          		  
        		   if((Hcount<=MaxBands)&&(TruncatedHeaders==1))
        		   	{
        		   		TruncatedBinary(Headers,b);
        		   		CR[0] = L2SB_MultiCompression(Bands,Headers);       		           
                   		if(CR[0]>CRmax[0]) { CRmax[0] = CR[0]; }
                   		if(CR[0]<CRmin[0]) { CRmin[0] = CR[0]; }
                 	}
                 	
                   if((Hcount<=MaxBands)&&(UniformHeaders==1))
        		    {
                   		UniformBinary(Headers,b);
                   	    CR[1] = L2SB_MultiCompression(Bands,Headers);       		           
                   		if(CR[1]>CRmax[1]) { CRmax[1] = CR[1]; }
                   		if(CR[1]<CRmin[1]) { CRmin[1] = CR[1]; }
                   	}
	
                //////////////////////////////////////
                // outout progress for monitoring
                     
                   if(Hcount<=MaxBands)
                   	{   
                   		printf("\r XPERM %03d (H%d,%d) ) [T=d%%][ ",permcount,Hwidth,Hcount);
                
                   		for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Bands[i]); }
                   		printf(" -- ");
                   		for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Headers[i]); }
                	
                   		printf("] ");
                   
                   		if(TruncatedHeaders==1) 
                   		{ printf(" TB: CR[%5.3f] Min[%5.3f] 	Max[%5.3f]",CR[0],CRmin[0],CRmax[0]); }
                   			
                   		if(UniformHeaders==1)   
                   		{  printf(" UB: CR[%5.3f] Min[%5.3f] Max[%5.3f]",CR[1],CRmin[1],CRmax[1]); }
                             
                			if(ResultsFile==1)
                			{
                	   			if(TruncatedHeaders==1) 
                	   			{
                	   				if(hop!=NULL)
                					{ fprintf(hop,"TB%03d.{",permcount); 
                	   				  for(int i=0;i<b;i++) 
                	   				  	{ if(Bands[i]<0){break;}
                	   				  	  fprintf(hop,"%d-",Bands[i]); 
                	   				  	}  
                	   				  fprintf(hop,"},");
                	   				}
                	   				
                	   				fprintf(op,"%5.3f,",CR[0]);
                	   			}
                	   
                	   			if(UniformHeaders==1) 
                	   			{
                	   				if(hop!=NULL) 
                					{ 
                					  fprintf(hop,"UB%03d.{",permcount); 
                	   				  for(int i=0;i<b;i++) 
                	   				  	{ if(Bands[i]<0){break;} 
                	   				  	  fprintf(hop,"%d-",Bands[i]);  
                	   				  	}  
                	   				  fprintf(hop,"},");
                	   				}
                	   				
                	   				fprintf(op,"%5.3f,",CR[1]);
                	   			}
                			}                			
                			 
                }
                permcount++;
			}
			
			
        }
 		        		
    if(ResultsFile==1)
    {
     fclose(op);
     fclose(hop);
    }
    
    fclose(perms);
}


inline float L2SBCompression(int b1,int b2,int b3,int b4,int h1,int h2,int h3,int h4)
{
	int Band[32];
	int Header[32];
	
		Band[0]=b1;
		Band[1]=b2;
		Band[2]=b3;
		Band[3]=b4;
		Band[4]=-1;
		
		Header[0]=h1;
		Header[1]=h2;
		Header[2]=h3;
		Header[3]=h4;
		Header[4]=-1;
		
	return ( L2SB_MultiCompression(Band,Header) );
}

inline float L2SB_MultiCompression(int *Band,int *Headers)
{
    float CR = 0;
    long int BitCount =0;
    int Bits =0;
    int Header =0;
    int QPoint;
    
    char BinStr[64];
    char DiffStr[64];
    long int Difference;
    long int Count;
    
    int Debug=L2SBCompression_Debug;
    int test=0;
    
    ////////////////////////////////////////////
    // assume first word is full-length
    // and header is the Most Significant Band
    // item (item at end of list)
    	
    	
    	Quantisation=BitWidth;
    	
    	int H=31;
    	
    	for(int x=0;x<32;x++) 
    		{ if(Headers[x]>=0){ H=x;} else { break;}}
    		
        BitCount = Quantisation + Headers[H]; //+2;
          
          
     /////////////////////////////////////////////
     // debug info
          
        if(test==1){printf("\n");}
 		 		
 		 if(1)
 		 {  for(int x=0;x<32;x++)
 		 	{
 		 		if(test==1)printf("[%02d|%d] ",Headers[x],Band[x]);
 		 		if (Band[x+1]<0) {break;}
 		 	}
 		    if(test==1)printf("\n");
 		 }
 		 
 	
 	 ///////////////////////////////////////////////
 	 // compress data values in array
 		
        for(Count=1; Count < DataCount; Count++)
        {
          	if((Count%250000)==0)
          		{ printf("%ld\n", Count);
          			
          		}
          		          	
            //////////////////////////////////////////////
            // calculate difference
            
               Difference = Data[Count-1] ^ Data[Count];
               
                                
               for(int x=0;x<Quantisation; x++) { Difference = Difference | (Difference >>1); }        
                
                 
               DecToBin(BinStr,Data[Count],Quantisation);        
               DecToBin(DiffStr,Difference,Quantisation);        
              
              
              	 			
            //////////////////////////////////////////////
            // work out which band applies
            
               QPoint = Quantisation;            
               Header =-1;
                
               
                  
               //////////////////////////////////   
               /// FIND THE DIFFERENCE BOUNDARY 
               
                   int x;
                   
                   for(x=0;x<32;x++)
               	   {
               		 if(Band[x]<0) { break;}   // end of list detected, so exit loop.
               		
               		 if(test==1)
               		 { printf("QPoint %d (Bit %d)) %c.%s \n",QPoint,Quantisation-QPoint,DiffStr[QPoint-1],&DiffStr[QPoint]);
               		 }
               		 
               		 if(DiffStr[QPoint-1]=='0') { break; }  // if zero then no more bands to encode 
               		 
               		 Header = Headers[x];					// set new current header
               		 QPoint = QPoint - Band[x];   			// set new bit query position	
               	   }
               	
               	
               	//////////////////////////////////////////////
               	//  CALCULATE HOW MANY BITS ARE TO BE ENCODED
               	
               	    Bits=0;
               	
               	    for(int i=0; i<x; i++){ Bits += Band[i];}
               	
               	    BitCount +=  Bits + Header;
               	
               	////////////////////////////////////////////////////////////
               	// OPTIONAL DEBUG OUTPUT - limits data set to a small run	
               	
               	   if(test==1)
               	   { printf("b(%d) B[%s] D[%s] Q[%d] H[%02d] Bits[%d] T[%d]\n",x,BinStr,DiffStr,QPoint,Header,Bits, Bits+Header);
               	     printf("---------------------\n");

               	     if(Count==10){  break;  }
				   }               
        }
    
    	
    	
        CR = (double)(Quantisation * Count)  / (double)BitCount ;
    
        if(Debug)
        	printf("Raw Bits %ld , Compressed %ld %f\n",DataCount*Quantisation, BitCount, CR);
       
    return(CR);
}


inline void DecToBin(char * Bin, long int i, long  int n)
{
    char Tmp[1024]="";
    
    Bin[0]=0;
    
          
    for(long int x = 0; x<n; x++)
    {
        if( (i%2)==0) { sprintf(Tmp,"0%s",Bin); } else { sprintf(Tmp,"1%s",Bin); }
        strcpy(Bin,Tmp);
        i = i >>1;
        
        //printf("%d-%d-%d\n",i,n,x);
        
        // printf("%d .. %d -- %d \n",x,i,n);
    }

	
}

inline void ConfigTestCase(int Selection)
{

    switch(Selection)
    {
        case TEST_CONFIG_BONN_EEG:
            
                 DataRange = 1000.0;   // 1000 uv for seizure, 100uv for other data
                 Quantisation = BitWidth;
        
                 break;
                 
        case TEST_CONFIG_MITBIH_ECG:
            
                 DataRange = 2048;   // 1000 uv for seizure, 100uv for other data
                 Quantisation = BitWidth;
        
                 break;
        case TEST_CONFIG_DEFAULTS:
        
                 break;
                 
        default: 
        
                 DataRange = 16.0;
                 Quantisation = 12;
    }
    
    printf("--------------------------------\n");
    printf("-- CONFIG ----------------------\n");
    printf(" Data Range %5.2f\n",DataRange);
    printf(" Quantisation %d bits\n",Quantisation);
    printf("--------------------------------\n");
    
}

inline void RPerms(int n)
{
	static int c=-2;
	static int B[32];
    static int Index=B[0];
    static int permcount=0;
    static FILE * op = NULL;
    
    if(op==NULL)
    	{
    		op=fopen("perms.txt","w");
    		if(op==NULL){ printf("PERM FILE ERROR \n"); exit(0); }
    	}
    	
    c++;
    permcount++;
    
	if(c>50) { printf("Call Depth Exceeded \n"); exit(0); }
		
	if(n>0) { Index=1; B[1]=n; permcount=0; c=-2; RPerms(0);  return; }
	
	if(0)
	{	
	  printf("-%03d- perm.%03d ",c,permcount );
	  for(int i=1; i<=Index; i++) { printf("[%02d] ", B[i]); }
	  printf("\n");
	}
	
	for(int i=1; i<=Index; i++) { fprintf(op,"%d,", B[i]); }
	fprintf(op,"-1\n");
			
	Index++; 
	B[Index]=0;
	
	while(B[Index-1]>1) { B[Index-1]--;  B[Index  ]++; RPerms(0); }
	
	B[Index-1] = B[Index-1] +B[Index];	
	Index--;	
	c--;
	
	// only executed on final return.
	if(c==-2)  
		{  
		   printf(" DataLength: %d, Permutations: %d\n\n",B[1],permcount); 
		   fclose(op);
		}
		
	
}

inline void TruncatedBinary(int * Headers, int n)
{
	
    int Hcount = n;
    int Hwidth =  (log2(Hcount-1)+1);
    int FullRange = pow(2,Hwidth);
    int Unused = FullRange - Hcount;
    char Str[32];
    
    //printf("n=%d, b=%d, R=%d, U=%d ",Hcount,Hwidth,FullRange, Unused);
    
    for(int i = 0; i < Hcount; i++)
    	{
    		if(i < Unused)  { Headers[i]= Hwidth-1; } //DecToBin(Str,i, Hwidth-1); }
    		if(i >= Unused) { Headers[i]= Hwidth;   } //DecToBin(Str,i+Unused,Hwidth); }
    		//printf("%s-",Str);
    	}
    
    
    //printf("\n");
	
}

inline void UniformBinary(int * Headers, int n)
{
	
	int Hcount = n;
    int Hwidth =  (log2(Hcount-1)+1);
   
    
    for(int i = 0; i < Hcount; i++)
    	{
    		Headers[i]= Hwidth; 
       	}
    	
}

inline void SetMaxBands(int x)
{
	MaxBands=x;	
	
	if(MaxBands<1) { MaxBands=BitWidth;}
}

void SetTruncatedHeaders(int x)
{
	TruncatedHeaders =x;
}

void SetUniformHeaders(int x)
{
	UniformHeaders=x;
}

void SetFileStep(int x)
{
	FileStep=x;
}

void SetDataLimit(long int x)
{
	DataLimit = x;
}



void SetBitWidth(int n)
{
	BitWidth=n;
}

inline void SetOutputFile(char* Str)
{
	strcpy(OutputFile,Str);
}

inline void SetHeaderFile(char* Str)
{
	strcpy(HeaderFile,Str);
}

void SetUnipolarData()
{
	Unipolar=1;
}

void SetBipolarData()
{
	Unipolar=0;
}

void SetShowQuantisation(int x)
{
	ShowQuantisation=x;
}

void SetResultsOutput(int x)
{
	ResultsFile = x;	
}

inline void ReportConfig()
{
	////////////////////////////////////////
	// report settings
	
		printf(" Number of bands allowed %d \n",MaxBands );
		printf(" File Step %d \n",FileStep);
		
		if(TruncatedHeaders) { printf(" Run Includes Truncated Headers\n"); }
		if(UniformHeaders)   { printf(" Run Includes Uniform Headers\n");   }
		
		if(DataLimit>0)		 { printf(" Data File Limit set to %ld samples\n",DataLimit); }
		if(Unipolar==1)		 { printf(" Unipolar Data Mode \n"); }
		if(Unipolar==0)		 { printf(" Bipolar Data Mode \n"); }

		
		printf(" Output File %s\n",OutputFile);
		printf(" Header File %s\n",HeaderFile);
}

float CRUB=0;
float CRTB=0;

inline float test_L2SB(char * BandConfig)
{
    float CR[2]={0,0};   
    int Q = Quantisation;
    int permcount=0;
    float CRmax[2] ={0,0};
    float CRmin[2] ={999,999};       
    
    FILE * op;
    FILE * hop;
    
    if(ResultsFile==1)
    {
    	op = fopen(OutputFile,"a");
    	if(op==NULL) { printf("Output file error \n"); return(-1); }
   
    	hop = fopen(HeaderFile,"r");
    	if(hop==NULL) { hop = fopen(HeaderFile,"w"); }
   	  	else { hop = NULL; }
    	fprintf(op,"\n%s/%s,",FilePath,FileName);
    }
    
     
        
    
        if(1)
        {    char Line[1024];
        	 int Bands[32];
    		 int Headers[32];
    		 int b=0;
    		 
        	
        			sprintf(Line,"%s",BandConfig);
             			
             			b=0;
        			while(strstr(Line,",")!=NULL)
        				{
        					char * x = strstr(Line,",");
        					
        					x[0]=0;
        					Bands[b++]=atoi(Line);
        					strcpy(Line,&x[1]);
        				}
        				
        				Bands[b++]=atoi(Line);
        				Bands[b]=-1;
        				
        			//for(int i=0;i<b;i++) { printf("%02d - ",Bands[i]); }
        			
        		
        		int Hcount = b;
        		int Hwidth =  (log2(Hcount)+1);
        		
        		if(Hwidth < 0){ Hwidth = 0;}
      
         		////////////////////////////////////////
        		// compression with truncated headers
        		  
        		    if(Hcount<=MaxBands)
                   	    {   
                   		printf("\r YPERM %03d (H%d,%d) ) BANDS: ",permcount,Hwidth,Hcount);
                	   
                   	        for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Bands[i]); }
                   	        printf(" -- ");
                   	    }  
                   	    
        		   if((Hcount<=MaxBands)&&(TruncatedHeaders==1))
        		   	{
        		   	   TruncatedBinary(Headers,b);
        		   	   
        		   	   printf("TH ");
        		   	   for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Headers[i]); }
        		   	   
        		   	   CR[0] = L2SB_MultiCompression(Bands,Headers);       		           
                   		   if(CR[0]>CRmax[0]) { CRmax[0] = CR[0]; }
                   		   if(CR[0]<CRmin[0]) { CRmin[0] = CR[0]; }
                 		}
                 	
                 	   
                   	   
                   	   
                   	   if((Hcount<=MaxBands)&&(UniformHeaders==1))
        		      {
                   		  UniformBinary(Headers,b);
                   		  printf("UH: ");
                   		  for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Headers[i]); }
                   		  
        		   	  CR[1] = L2SB_MultiCompression(Bands,Headers);       		           
                   		  if(CR[1]>CRmax[1]) { CRmax[1] = CR[1]; }
                   		  if(CR[1]<CRmin[1]) { CRmin[1] = CR[1]; }
                   	      }
	
                //////////////////////////////////////
                // outout progress for monitoring
                     
                   if(Hcount<=MaxBands)
                   	{   
                   		//printf("\r YPERM %03d (H%d,%d) ) [T=d%][ ",permcount,Hwidth,Hcount);
                
                   		//for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Bands[i]); }
                   		//printf(" -- ");
                   		//for(int i=0;i<b;i++) { if(Bands[i]<0){break;} printf("%d, ",Headers[i]); }
                	
                   		//printf("\n");
                   
                   		if(TruncatedHeaders==1) 
                   		{ printf(" TB: CR[%5.3f] Min[%5.3f] Max[%5.3f]",CR[0],CRmin[0],CRmax[0]); }
                   		if(UniformHeaders==1)   
                   		{ printf(" UB: CR[%5.3f] Min[%5.3f] Max[%5.3f]",CR[1],CRmin[1],CRmax[1]); }
                                printf("\n");
                                
                		if(ResultsFile==1)
                			{
                	   			if(TruncatedHeaders==1) 
                	   			{
                	   				if(hop!=NULL) 
                					{ fprintf(hop,"TB%03d.{",permcount); 
                	   				  for(int i=0;i<b;i++) { if(Bands[i]<0){break;} 
                	   				  fprintf(hop,"%d-",Bands[i]); }  
                	   				  fprintf(hop,"},");
                	   				}
                	   				
                	   				fprintf(op,"%5.3f,",CR[0]);
                	   			}
                	   
                	   			if(UniformHeaders==1) 
                	   			{
                	   				if(hop!=NULL) 
                					{ 
                					  fprintf(hop,"UB%03d.{",permcount); 
                	   				  for(int i=0;i<b;i++) { if(Bands[i]<0){break;} 		
                	   				  fprintf(hop,"%d-",Bands[i]); }  
                	   				  fprintf(hop,"},");
                	   				}
                	   				
                	   				fprintf(op,"%5.3f,",CR[1]);
                	   			}
                			}                			
                			 
                }
             
			
			
        }
        	
   
	if(ResultsFile==1)
	{	        		
      fclose(op);
      fclose(hop);
    }
    
      
    //float CRX=0;
    //
    //if(TruncatedHeaders==1) {CRX = CR[0];} else {CRX= CR[1];}
    //
    
    CRTB = CR[0];
    CRUB = CR[1];
    
    return(0);	
   }
