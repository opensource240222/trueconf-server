#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <map>

//#include "src/VS_Containers.h"


#define MAX_NAME_SIZE 30000

/////////////////////////////////////////////////////////////////////////////////////////
extern const char pream[];

extern const char string_types[][MAX_NAME_SIZE];

extern int dddd;
///scanf("%s",&dddd);
#define l(x) printf("\n\t%s",x);
#define lo(x) printf("\n\t%s",x);
#define info(x) printf("\n\t%s",x);
/////////////////////////////////////////////////////////////////////////////////////////

extern const char seq_r[];
extern const char seq_b[];
extern const char seq_e[];
extern const char seq_x[];

#define DError( x , y ) {printf("\n\t Error in %s res=%d", x , y ); return 0;}

/////////////////////////////////////////////////////////////////////////////////////////
enum VS_AsnSimpleTypes
{ e_undefined,
e_typeid,
e_hashed,
e_encrypted,
e_signed,
e_null,
e_bool,
e_integer,
e_enumeration,
e_objectid,
e_bitstring,
e_octstring,
e_numstring,
e_ia5string,
e_pristring,
e_genstring,
e_bmpstring,
e_choice,
e_sequence,
e_set,
e_sequence_of,
e_set_of
};
enum  VS_AsnComplerSpecialToken
{
	e_optional,
	e_equal,
	e_array,
	e_dots,
	e_comma
};
/*
enum VS_AsnCompilerModes
{
	e_undef,
	e_in_container,
};
*/
extern char prefix[MAX_NAME_SIZE];

template <typename Type, typename KeyType, int numbers> struct VS_AsnCompilerNameT
{
public:
	VS_AsnCompilerNameT()  { memset(data,0,sizeof(Type)*numbers);  }
	Type data[numbers];
	void operator=(const VS_AsnCompilerNameT< Type , KeyType , numbers > &src)
	{
		memcpy(data,src.data,numbers);
	}
	bool operator==(const VS_AsnCompilerNameT< Type, KeyType, numbers > &Right)
	{
		return !memcmp(&data,&Right.data,sizeof(Type)*numbers);
	}
	/*bool operator < (VS_AsnCompilerNameT< Type, KeyType, numbers > &Right)
	{
		int i = memcmp(data,Right.data,sizeof(Type)*numbers);
		if(i>0)
			return true;
		else
			return false;
	}*/

	VS_AsnCompilerNameT(const VS_AsnCompilerNameT&) = delete;


};

/////////////////////////////////////////////////////////////////////////////////////////
enum  VS_AsnCompilerRangeType{
	e_one_digit,
	e_range,
	e_extended_range};
struct VS_AsnCompilerRange
{
	VS_AsnCompilerRangeType rangeType;
	int low;		  /// Если задан размер элемента - нижний размер, иначе 0.
	unsigned long upp; /// Если задан размер элемента - нижний размер, иначе 0.
	bool isExtendable;/// Флаг расширяемости для размера элемента.
	VS_AsnCompilerRange()
	{	low = upp = isExtendable = 0;}
	void Init()
	{
		low = 0;
		upp = 0;
		isExtendable = 0;
		rangeType = e_range;
	}
	void Show()
	{
		printf("\n\t Range \tlow:\t%d",low);
		printf("\n\t Range \tupp:\t %lu",upp);
		printf("\n\t Range \trangeType:\t %d",rangeType);
		printf("\n\t Range \tisExtendable:\t %d",isExtendable);

	}
};

/////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned int VS_AsnCompilerKey;
/////////////////////////////////////////////////////////////////////////////////////////
typedef VS_AsnCompilerNameT<char,unsigned int, MAX_NAME_SIZE>  VS_AsnCompilerName;/// ключ

//typedef char*  VS_AsnCompilerKey;
/////////////////////////////////////////////////////////////////////////////////////////
typedef short int VS_AsnCompilerConstraienType;
/////////////////////////////////////////////////////////////////////////////////////////
extern const unsigned long DEFAULT_SIZE;
/////////////////////////////////////////////////////////////////////////////////////////
VS_AsnCompilerName VS_AsnCompilerVarNameCorrector(VS_AsnCompilerName sname);
/////////////////////////////////////////////////////////////////////////////////////////
VS_AsnCompilerName VS_AsnCompilerNameCorrector(VS_AsnCompilerName sname);
/////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnCompilerNameTranslator( VS_AsnCompilerName name ,VS_AsnCompilerName &vs_name );
/////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnCompilerNameTranslator(VS_AsnCompilerName &name);
/////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnCompilerEmbededNameTranslator(VS_AsnCompilerName &name , VS_AsnCompilerName fathers_name);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
struct VS_AsnCompilerBuffer
{
	VS_AsnCompilerBuffer()
	{
		buf.reserve( DEFAULT_SIZE );
		ptr_buf = &buf[0];
		index  =  &buf[0];
		i=0;
	}
	std::vector< char> buf;
	std::vector< char>::pointer ptr_buf;
	std::vector< char>::pointer index;
	unsigned int i;
	void AddString(const char * in,unsigned long length=0)
	{
		if (!in) {printf("\n\t in = 0 in AddString");return ;}
		unsigned long len = length;
		if (len==0) len = strlen(in)+1;
		unsigned long cap = buf.capacity();
		unsigned long siz = buf.size();
		unsigned long rsiz = buf.size();
		//printf("\n len %d cap:%d size: %d",len,cap,siz);
		//return;
		siz+=len;
		while ( (siz) > cap )
		{
			buf.reserve( cap + DEFAULT_SIZE);
			cap = buf.capacity();
		}

		buf.resize( siz -1 );
		for(i=rsiz;i<siz -1;i++) buf[ i ] = in[i - rsiz];
		//snprintf(ptr_buf,len,"%s",in);
		//printf("\n in: %s", in);
		//printf("\n tout: %s", ptr_buf);
		//printf("\n REAL size:%d %d",buf.size(), siz);
		//ptr_buf +=len;
		//--ptr_buf;
		//ptr_buf +=len;
	}
	unsigned long GetSize()
	{
		return buf.size();
	}
	void GetString(char strings[])
	{
		index  =  &buf[0];
		unsigned long size = buf.size();
		//printf("\n UNREAL GET size: %d",size);
		//FILE * ppp = fopen("buf.log","w");
		//for( i=0;i<size;i++) fprintf(ppp,"%d\t%c\t%d\n",i,buf[i],buf[i]);
		snprintf(strings,size+1,"%s",index);
		//printf("\n get out : %s",strings);
	}
	void Show()
	{
		unsigned long size = buf.size();
		printf("\n\t THIS  : %p",this);
		printf("\n\t index : %p",index);
		printf("\n\t size  : %lu",size);
		printf("\n\t buffer: %p",ptr_buf);
		printf("\n\t ptr   : %p",&buf);
	}
	int SaveToFile(FILE * out)
	{
		//return fwrite(index,1,buf.size(),out);
		for(i=0;i<buf.size();i++)
		{
			fprintf(out,"%c",buf[ i ]);
			//printf("%c",buf[ i ]);
		}
		return 1;
	}


};

/////////////////////////////////////////////////////////////////////////////////////////

/// описатель типов.
struct VS_AsnTypeCompilerContainer
{
	////////////////////////////////////

	VS_AsnSimpleTypes type;/// Тип элемента, вне зависимости от размера его массива и ег размара.
	VS_AsnSimpleTypes arrayType; /// Тип массива.
	VS_AsnCompilerRange typeRange;
	VS_AsnCompilerRange arrayRange;
	bool isOptional;			 /// Флаг optional - ности элемента.
	//VS_Asn::ConstraintType constrained;
	VS_AsnCompilerConstraienType constrainedArray;
	VS_AsnCompilerConstraienType constrainedType;
	bool isEmbeded;               /// тип отвечает  -  вложенный ли это обьект.
	bool isTypeSizeExist;		//< флаг присутствия размера в типе
	bool isArraySizeExist;		//< флаг присутствия размера в массиве типа
	bool isArrayExist;			//< флаг присутствия МАССИВА ТИПА
	bool isMetaTypeSimple;		//< флаг указывает что мета тип простой(в 1 строку)
	bool isTypeBad;
	/// ( напримр, sequence вложеный в другой sequence).
	VS_AsnCompilerName simple_meta_name;/// для typedef - oB
	VS_AsnCompilerName vs_name;     //< имя класса в в нашей системе
	VS_AsnCompilerName typeName;    /// имя класса в Asn
	VS_AsnCompilerName fathersName; /// имя отца, если isEmbeded=true;
	VS_AsnCompilerKey  myKey;		//< КЛЮЧ в дереве внутреннего представления
	VS_AsnCompilerKey  fathersKey;  //< КЛЮЧ ОТЦА в дереве внутреннего представления для вложенных типов
	////////////////////////////////
	VS_AsnCompilerName alphabet;	//< алфавит для Ia5 строк
	bool isAlphabetExist;			//< флаг присутствия алфавита для Ia5 строк
	////////////////////////////////
	int  res;
	char tmp[MAX_NAME_SIZE];
	inline void ZeroTmp()
	{
		memset(tmp,0,MAX_NAME_SIZE);
	}
	////////////////////////////////
	void SetBad()
	{
		isTypeBad = true;
	}
	void Init()
	{
		type = e_undefined;
		arrayType = e_undefined;
		typeRange.Init();
		arrayRange.Init();
		isOptional = 0;
		constrainedArray = 0;
		constrainedType = 0;
		isEmbeded = false;
		isTypeSizeExist = false;
		isArraySizeExist = false;
		isArrayExist = false;
		myKey = 0;
		fathersKey = 0;
		ZeroTmp();
		isTypeBad = false;
		isAlphabetExist = false;

	}
	////////////////////////////////////
	void Show()
	{
		printf("\n\t type:\t%d",type);
		printf("\n\t arrayType:\t %d",arrayType);
		printf("\n\t typeRange:\t %d",typeRange.rangeType);
		typeRange.Show();
		printf("\n\t arrayRange:\t %d",arrayRange.rangeType);
		arrayRange.Show();
		printf("\n\t isOptional:\t %d",isOptional);
		printf("\n\t constrainedArray:\t %d",constrainedArray);
		printf("\n\t constrainedType:\t %d",constrainedType);
		printf("\n\t isEmbeded:\t %d",isEmbeded);
		printf("\n\t vs_name:\t %s",vs_name.data);
		printf("\n\t typeName:\t %s",typeName.data);
		printf("\n\t simpleName:\t %s",simple_meta_name.data);
		printf("\n\t fathersName:\t %s",fathersName.data);
		printf("\n\t myKey:\t %d",myKey);
		printf("\n\t fathersKey:\t %d",fathersKey);
		printf("\n\t isTypeSizeExist:\t %d", isTypeSizeExist );
		printf("\n\t isArraySizeExist:\t %d", isArraySizeExist);
		printf("\n\t isArrayExist:\t %d", isArrayExist);

	}
	/////////////////////////////////////////////////////////////////////////////////////////

	inline int MTypeName_type(VS_AsnCompilerBuffer &buffer, VS_AsnCompilerName &varName, char mode)
	{	ZeroTmp();
		//lo("\n+++++++++++++++++++++++++++++++++++++++\n");
		//printf("\t TypeName:%s Type :%d varName:%s",typeName.data,type,varName.data);
		if (type==e_undefined)
		{
			if (isTypeSizeExist)
			{
				printf("\n ! ERROR : unknown type size!\n");
				printf("\n\t TYPE: %s", typeName.data);
				//printf("\n\t VARN: %s", varName.data);
				VS_AsnTypeCompilerContainer::Show();
				FILE * out = fopen("critiicle.cpp","w");
				char buf[100000] = {0};
				buffer.GetString( buf );
				fprintf(out," %s ",buf );
				fclose( out );
				exit(0);
			}
			//LAST EDITION///
			VS_AsnCompilerName tmpName;
			tmpName = typeName;
			//if (!(isMetaTypeSimple&&(type==e_undefined)))
			VS_AsnCompilerNameTranslator( tmpName );
			snprintf(tmp,MAX_NAME_SIZE," %s ",tmpName.data );
			///LAST ED END///

			buffer.AddString( tmp );
			return 1;
		}
		///////////////////
		if (type >= e_sequence_of)
		{
			if (isTypeSizeExist)
			{
				printf("\n ! ERROR : unknown type size!\n");
				printf("\n\t TYPE: %s", typeName.data);
				VS_AsnTypeCompilerContainer::Show();
				FILE * out = fopen("critiicle.cpp","w");
				char buf[100000] = {0};
				buffer.GetString( buf );
				fprintf(out," %s ",buf );
				fclose( out );
				exit(0);
			}
			snprintf(tmp,MAX_NAME_SIZE," %s ",simple_meta_name.data );
			buffer.AddString( tmp );
			return 1;


		}
		///////////////////
		switch(type)
		{
		case e_typeid:
			{
						snprintf(tmp,MAX_NAME_SIZE,"Type_id< %s > ",
							typeName.data);
						buffer.AddString( tmp );
						return 1;
			}break;
		case e_hashed:
			{
						snprintf(tmp,MAX_NAME_SIZE,"VS_H235HASHED< %s > ",
							typeName.data);
						buffer.AddString( tmp );
						return 1;
					  }break;
		case e_encrypted:
			{
						snprintf(tmp,MAX_NAME_SIZE,"VS_H235ENCRYPTED< %s > ",
							typeName.data);
						buffer.AddString( tmp );
						return 1;
					  }break;
		case e_signed:
			{
						snprintf(tmp,MAX_NAME_SIZE,"VS_H235SIGNED< %s > ",
							typeName.data);
						buffer.AddString( tmp );
						return 1;
					  }break;
		case e_integer:
				{
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplInteger<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}

				}break;
		case e_bitstring:
				{
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplBitString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplBitString< 0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}

				}break;
		case e_octstring:
			{
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplOctetString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}

			}break;
		case e_numstring:
			{	//TemplNumericString
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplNumericString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplNumericString< 0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}
					//printf("\n\t ERROR : Numeric string have not released yet!\nAsk Alex.");
					//exit(0);
			}break;
		case e_ia5string:
			{

					if(isAlphabetExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplAlphabeticString< %s_alphabet, %s_alphabet_size,%s_inverse_table,",
							varName.data,varName.data,varName.data );
						buffer.AddString( tmp );

					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplIA5String<");
						buffer.AddString( tmp );
					}
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}
			}break;
		case e_pristring:
			{
					snprintf(tmp,MAX_NAME_SIZE,"TemplPrintableString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
					buffer.AddString( tmp );

					return 1;
			}break;
		case e_genstring:
			{
				if(isTypeSizeExist)
				{
					printf("ERROR: General String isTyprSizeExist. Ask Alex or Matvey");
					exit(0);
				}
				snprintf(tmp,MAX_NAME_SIZE,"VS_AsnGeneralString ");
				buffer.AddString( tmp );
				return 1;
			}break;
		case e_bmpstring:
			{
					if (isTypeSizeExist)
					{
						snprintf(tmp,MAX_NAME_SIZE,"TemplBmpString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
						typeRange.low,typeRange.upp,typeRange.isExtendable);
						buffer.AddString( tmp );
						return 1;
					}
					else
					{
						//printf("\n\t Error in MTypeName_type - e_bmpstring -not implemented");
						//return 0;
						snprintf(tmp,MAX_NAME_SIZE,"TemplBmpString<0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return 1;
					}
			}break;
		default:
			{
					if ((e_undefined==type)||(type > e_set_of ))
					{
						printf("\n\t ERROR bad sized type!\n");
						printf("\n\t varname : %s type:%d TypeName: %s ",
						varName.data, type , typeName.data);
						exit(0);
					}
					else
					{
        				snprintf(tmp,MAX_NAME_SIZE," %s ",string_types[ type ]	);
						buffer.AddString( tmp );
					}
			}
		}
		return 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	inline int MTypeName_size(VS_AsnCompilerBuffer &buffer, char mode)
	{	ZeroTmp();
		if (this->isArraySizeExist)
		{
			snprintf(tmp,MAX_NAME_SIZE,",%d,%lu,VS_Asn::FixedConstraint,%d ",
			arrayRange.low,
			arrayRange.upp,
			arrayRange.isExtendable);
			buffer.AddString( tmp );
		}
		else
		{
			snprintf(tmp,MAX_NAME_SIZE,",0,INT_MAX,VS_Asn::Unconstrained,%d ",
			arrayRange.isExtendable);
			buffer.AddString( tmp );
		}
		return 1;
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	inline int MTypeName(VS_AsnCompilerBuffer &buffer, VS_AsnCompilerName &varName , char mode)
	{	ZeroTmp();
		//lo("\n---------------------------\n");
		//printf("\t 1TypeName:%s Type :%d varName:%s",typeName.data,type,varName.data);
		if (isMetaTypeSimple&&(type==e_undefined))
		{
			varName = typeName;
		}
		//lo("\n---------------------------\n");
		//printf("\t 2TypeName:%s Type :%d varName:%s",typeName.data,type,varName.data);
		if ((isEmbeded==true))
		{	//	lo("type==e_undefined ");
			if (isTypeSizeExist)
			{
				printf("\n ! ERROR : unknown type size!\n");
				printf("\n\t TYPE: %s", typeName.data);
				//printf("\n\t VARN: %s", varName.data);
				VS_AsnTypeCompilerContainer::Show();
				FILE * out = fopen("critiicle.cpp","w");
				char buf[100000] = {0};
				buffer.GetString( buf );
				fprintf(out," %s ",buf );
				fclose( out );
				exit(0);
			}
			else
			{
				//LAST EDITION///
				//VS_AsnCompilerName tmpName;
				//tmpName = typeName;
				//VS_AsnCompilerNameTranslator( tmpName );
				//snprintf(tmp,MAX_NAME_SIZE," %s\t ",tmpName.data );
				//WAS//
				snprintf(tmp,MAX_NAME_SIZE,"%s\t",typeName.data );
				///LAST ED END///
				buffer.AddString( tmp );
				return 1;
			}
		}
		else
		{	//lo("type==e_undefined - else");
			if (isArrayExist)
			{		//lo("type==e_undefined - else1");
					buffer.AddString( "Constrained_array_of_type< " );

					res = MTypeName_type( buffer , varName ,  mode );
					//printf("\n\t Res=%d",res);
					if (res==1)	res = MTypeName_size( buffer , mode );
					else
					{
						DError("1 MTypeName - if (this->isArraySizeExist)",res);
						//printf("\n\t Error in  res=%d",res);
						//return 0;
					}
					if (res==1)	buffer.AddString( " > " );
					else
					{
						DError("2 MTypeName - if (this->isArraySizeExist)",res);
					}
					return 1;
			}
			else
			{
				//lo("type==e_undefined - else - else");
				res = MTypeName_type( buffer , varName ,  mode );
				//printf("\n\t Res=%d",res);
				if (1==res)	return 1;
				else
				{
					DError("MTypeName - if () else ",res);
				}
				return 1;
			}
		}
		DError("MTypeName - if () otherwise!!! ",res);

	}
	/////////////////////////////////////////////////////////////////////////////////////////
	int MHEADERAlphabet( VS_AsnCompilerBuffer &buffer,
						 VS_AsnCompilerName &name,
						 VS_AsnCompilerName &meta_name,
						 char mode)
	{	ZeroTmp();
		if (!isAlphabetExist) return -1;
		snprintf(tmp,MAX_NAME_SIZE,"\n\t ///////////////////////////////////////////////////////////////////////////////////////// \n");
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char   %s_alphabet[];\n",name.data);
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned		%s_alphabet_size;\n",name.data );
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char   %s_inverse_table[];\n",name.data);
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\t static const bool      %s_flag_set_table;\n",name.data);
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\n\t ///////////////////////////////////////////////////////////////////////////////////////// \n");
		buffer.AddString( tmp );

		return 1;
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	int MCPPAlphabet( VS_AsnCompilerBuffer &buffer,
						 VS_AsnCompilerName &name,
						 VS_AsnCompilerName &meta_name,
						 char mode)
	{		ZeroTmp();
			if (!isAlphabetExist) return -1;
			snprintf(tmp,MAX_NAME_SIZE,"\n\t ///////////////////////////////////////////////////////////////////////////////////////// \n");
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\tunsigned char %s::%s_alphabet[]=\n\t{",meta_name.data,name.data);
			buffer.AddString( tmp );
			unsigned i = 0;
			unsigned limit = strlen( alphabet.data );
			for(i=0;i<limit;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"'%c'",alphabet.data[i]);
				buffer.AddString( tmp );
				if (i+1 != limit)
					buffer.AddString( "," );
			}
			buffer.AddString( " };\n" );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\tunsigned  %s::%s_alphabet_size=sizeof(%s::%s_alphabet);\n"
				,meta_name.data,name.data,meta_name.data,name.data);
			buffer.AddString( tmp );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\tunsigned char  %s::%s_inverse_table[256]={0};\n",meta_name.data,name.data);
			buffer.AddString( tmp );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\t const bool %s::%s_flag_set_table = \n",meta_name.data,name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t VS_AsnRestrictedString::MakeInverseCodeTable(\n");
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_inverse_table,\n",meta_name.data,name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_alphabet,\n",meta_name.data,name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_alphabet_size );\n",meta_name.data,name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\t ///////////////////////////////////////////////////////////////////////////////////////// \n");
			buffer.AddString( tmp );
		return 1;
	}

/////////////////////////////////////////////////////////////////////////////////////////

};


/////////////////////////////////////////////////////////////////////////////////////////
	/// например, одной строки в случае простого или внешнего класса.
struct VS_AsnPartCompilerContainer : public VS_AsnTypeCompilerContainer /// описатель под части некоторого класса Asn.
{
	VS_AsnCompilerName varName;/// имя переменной
	//VS_AsnTypeCompilerContainer typeDecribe;/// описатель типа для переменной.
	void Show()
	{
		VS_AsnTypeCompilerContainer::Show();
		printf("\n\t varName:\t %s",varName.data);
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	int  MakeLine(VS_AsnCompilerBuffer &buffer )
	{		ZeroTmp();
			VS_AsnCompilerName tmpName;
			tmpName = varName;
			buffer.AddString("\t");
			res = MTypeName( buffer , tmpName, 0 );
			//printf("\n\t VAR1:%s",varName.data);
			if (res!=1) {DError("MakeLine - MTypeName",res);}
			///////
			snprintf(tmp,MAX_NAME_SIZE," %s ;\n ",varName.data);
			//printf("\n\t VAR2:%s",varName.data);
			buffer.AddString( tmp );
			//Show();
			//int d;
			//scanf("%d",&d);
			return 1;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/// описатель класса Asn.
#define init_lines_size 2
struct VS_AsnMetaTypeCompilerContainer : public VS_AsnTypeCompilerContainer
{
	VS_AsnMetaTypeCompilerContainer()
	{	Init();
	}
	void Init()
	{
		numbers_of_lines = lines.begin();
		lines.resize( init_lines_size );
		line_size = init_lines_size;
		current_line = 0;
		walk_line = 0;
		isWalkDown = false;
		num_of_extension = 0;
		num_of_options = 0;
		num_of_simple = 0;
		VS_AsnTypeCompilerContainer::Init();
		isExtendable = false;
		ZeroTmp();
	}
	bool isExtendable;
	unsigned long num_of_simple;
	unsigned long num_of_options;
	unsigned long num_of_extension;
	unsigned long line_size;
	unsigned long current_line;
	unsigned long walk_line;
	bool isWalkDown;

	std::vector< VS_AsnPartCompilerContainer > lines;
	std::vector< VS_AsnPartCompilerContainer >::iterator numbers_of_lines;


///////////////////////////////////////////////////////////////////////////////////////
inline int MakeClass(VS_AsnCompilerBuffer &buffer , VS_AsnCompilerBuffer &byffer_cpp);

inline int MakeHeadersStructs(VS_AsnCompilerBuffer &buffer , VS_AsnCompilerBuffer &byffer_cpp);
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

inline int MHEADERPreambule_sequence( VS_AsnCompilerBuffer &buffer );

inline int MCPPPreambule_sequence( VS_AsnCompilerBuffer &byffer_cpp);
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERPreambule_choice( VS_AsnCompilerBuffer &buffer );

inline int MCPPPreambule_choice( VS_AsnCompilerBuffer &byffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERPreambule_strings( VS_AsnCompilerBuffer &buffer );

inline int MCPPPreambule_strings( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERPreambule_simple( VS_AsnCompilerBuffer &buffer );

inline int MCPPPreambule_simple( VS_AsnCompilerBuffer &buffer );
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

inline int MHEADERConstructorDeclaration_strings( VS_AsnCompilerBuffer &buffer );

inline int MCPPConstructorRealisation_strings( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERConstructorDeclaration_choice( VS_AsnCompilerBuffer &buffer );

inline int MCPPConstructorRealisation_choice( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERConstructorDeclaration_sequence( VS_AsnCompilerBuffer &buffer );

inline int MCPPConstructorRealisation_sequence( VS_AsnCompilerBuffer &buffer_cpp );

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

inline int MCPPConstructorsBody_sequence( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MCPPConstructorsBody_choice( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MCPPConstructorsBody_strings( VS_AsnCompilerBuffer &buffer_cpp );

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

inline int MHEADERClassBody_sequence( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassBody_sequence( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERClassBody_choice( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassBody_choice( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERClassBody_strings( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassBody_strings( VS_AsnCompilerBuffer &buffer_cpp );

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

inline int MHEADERClassMethods_sequence( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassMethods_sequence( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERClassMethods_choice( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassMethods_choice( VS_AsnCompilerBuffer &buffer_cpp );
///////////////////////////////////////////////////////////////////////////////////////
inline int MHEADERClassMethods_strings( VS_AsnCompilerBuffer &buffer );

inline int MCPPClassMethods_strings( VS_AsnCompilerBuffer &buffer_cpp );

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
	/*
	inline void MakeDecodeChoice(VS_AsnPartCompilerContainer line , VS_AsnCompilerBuffer &buffer)
	{
				if (line.type==e_undefined)
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",line.varName.data);
					buf_cpp.AddString( tmp );
					VS_AsnCompilerName ttt;
					VS_AsnCompilerNameTranslator(line.typeName , ttt);
					snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new %s);\n",ttt.data);
					buf_cpp.AddString( tmp );
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",line.varName.data);
					buf_cpp.AddString( tmp );

					if (line.isAlphabetExist)
					{
						switch(line.type)
						{
						case e_ia5string:
							{
                                //TemplAlphabeticString
							snprintf(tmp,
							MAX_NAME_SIZE,
							"return DecodeChoice( buffer , new %s(",string_types[ lines[ i ].type ]);
							buf_cpp.AddString( tmp );

							}break;
						default:
							{
								printf("\n\t ERROR !!! isAlphabetExist in type:%d",line.type);
							}break;
						}

						if (lines[i].isTypeSizeExist)
						{
							snprintf(tmp
								,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d)",
								lines[i].typeRange.low,lines[i].typeRange.upp,lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp );
						}
						else
						{
							snprintf(tmp,
							MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,%d)",
							lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp );

						}
						snprintf(tmp,
							MAX_NAME_SIZE,
							");\n",string_types[ lines[ i ].type ]);
					}
					else
					{

						snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new ");//%s);\n",string_types[ lines[ i ].type ]);
						buf_cpp.AddString( tmp );
						lines[ i ].MakeType( buf_cpp );
						snprintf(tmp,MAX_NAME_SIZE," );\n");
					}
					buf_cpp.AddString( tmp );


				}

	}
	*/
	/////////////////////////////////////////////////////////////////////////////////////////
	void MakeWalk(struct VS_AsnCompilerContainer &container, VS_AsnCompilerBuffer &buffer , VS_AsnCompilerBuffer &byffer_cpp);

	void MakeShortWalk(struct VS_AsnCompilerContainer &container, VS_AsnCompilerBuffer &buffer , VS_AsnCompilerBuffer &byffer_cpp);
	/////////////////////////////////////////////////////////////////////////////////////////
	/*
	void MakePreambuleAlloc( char *& buffer, unsigned long & buf_size,
					char *& buffer_cpp,
			unsigned long & buf_cpp_size)

	{
		VS_AsnCompilerBuffer buf,buf_cpp;
		char tmp[MAX_NAME_SIZE]={0};
		char tmp_cpp[MAX_NAME_SIZE]={0};
		//////////////////////////////////////////////////////////////////////////////////////
		if ((type!=e_sequence) && (type!=e_choice) && (type!=e_set))
		{
			switch( type )
			{
			case e_ia5string:
			case e_numstring:
			case e_pristring:
			case e_genstring:
				{
					if (isAlphabetExist)
					{

			snprintf(tmp,MAX_NAME_SIZE,"struct %s : public VS_AsnRestrictedString\n{",typeName.data);
			buf.AddString( tmp );
			//// .HEADER
			snprintf(tmp,MAX_NAME_SIZE,"\t %s( void );\n",typeName.data);
			buf.AddString( tmp );
			//////////////

			snprintf(tmp,MAX_NAME_SIZE,"\t %s::%s( void ):\n",typeName.data,typeName.data);
			buf_cpp.AddString( tmp );
			snprintf(tmp,
			MAX_NAME_SIZE,"\t VS_AsnRestrictedString( %s_alphabet,\n\t\t%s_alphabet_size,\n\t\t%s_inverse_table,",
				typeName.data,typeName.data,typeName.data);

			buf_cpp.AddString( tmp );
			if (isTypeSizeExist)
			{
				snprintf(tmp,
					MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d)\n\t{",
					typeRange.low,typeRange.upp,typeRange.isExtendable);
			}
			else
			{
				snprintf(tmp,
					MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,%d)\n\t{",
					typeRange.isExtendable);

			}
			buf_cpp.AddString( tmp );
			this->MakeAlphabet( buf,buf_cpp,false,typeName,typeName);
			buf_cpp.AddString( "}");
			buf.AddString( "}");

					}
					else
					{
						snprintf(tmp,MAX_NAME_SIZE,"\ntypedef ");
						buf.AddString( tmp );
						if (isArrayExist)
						{
							MakeSize( buf );
						}
						else MakeType( buf );
					}
				}break;

			default:{
						snprintf(tmp,MAX_NAME_SIZE,"\ntypedef ");
						buf.AddString( tmp );
						if (isArrayExist)
						{
							MakeSize( buf );
						}
						else MakeType( buf );
						if (type==e_undefined) snprintf(tmp,MAX_NAME_SIZE," %s; ",simple_meta_name.data);
						else snprintf(tmp,MAX_NAME_SIZE," %s; ",typeName.data);
						buf.AddString( tmp );
					}break;
			}
			buf_size = buf.GetSize()+1;
			buffer = new char[buf_size];
			buf.GetString( buffer );
			buf_cpp_size = buf_cpp.GetSize()+1;
			buffer_cpp = new char[buf_cpp_size];
			buf_cpp.GetString( buffer_cpp );
			return ;
		}
		//////////////////////////////////////////////////////////////////////////////////////
		buf.AddString((char *)pream );
		snprintf(tmp,MAX_NAME_SIZE,"%s : public \0",typeName.data);
		buf.AddString( tmp );
		///.HEADER
		snprintf(tmp,MAX_NAME_SIZE," %s\n{\n\t%s( void );\n"
		,string_types[ type ],typeName.data);
		buf.AddString( tmp );
		///////////

		///.CPP
		snprintf(tmp_cpp,MAX_NAME_SIZE,"\n\t%s::%s( void ):\n\t%s( \0"
		,typeName.data,typeName.data,string_types[ type ]);
		buf_cpp.AddString( tmp_cpp );

		///////////


		switch( type )
		{
		case e_sequence:{
			///.CPP
			snprintf(tmp_cpp,MAX_NAME_SIZE,"%d , %s , %s, %s , %s , %d )\0",
				num_of_options,
				seq_r,
				seq_b,
				seq_e,
				seq_x,
				isExtendable);
				buf_cpp.AddString( tmp_cpp );
				unsigned i;
/*
				for(i=0;i<current_line;i++)
				{
					if (lines[i].isAlphabetExist)
					{
						snprintf(tmp_cpp
						,MAX_NAME_SIZE,",%s(%s_alphabet,\n\t\t%s_alphabet_size,\n\t\t%s_inverse_table,",
						lines[i].varName.data,lines[i].varName.data,lines[i].varName.data);
						buf_cpp.AddString( tmp_cpp );
						if (lines[i].isTypeSizeExist)
						{
							snprintf(tmp_cpp
								,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d)",
								lines[i].typeRange.low,lines[i].typeRange.upp,lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp_cpp );
						}
						else
						{
							snprintf(tmp_cpp,
							MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,%d)",
							lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp_cpp );

						}

					}
				}
				buf_cpp.AddString( "\n" );
*/
/*
						} break;
		case e_choice:	{
			///.CPP
			snprintf(tmp_cpp,MAX_NAME_SIZE,"%d , %d , %d )\n\0",
				num_of_simple,
				num_of_simple+num_of_extension,
				isExtendable);
			buf_cpp.AddString( tmp_cpp );
						}break;
		case e_set:		{
			///.CPP
			snprintf(tmp_cpp,MAX_NAME_SIZE,"%d , %s , %s, %s , %s , %d )\0",
				num_of_options,
				seq_r,
				seq_b,
				seq_e,
				seq_x,
				isExtendable);
				buf_cpp.AddString( tmp_cpp );
				unsigned i;
/*
				for(i=0;i<current_line;i++)
				{
					if (lines[i].isAlphabetExist)
					{
						snprintf(tmp_cpp
						,MAX_NAME_SIZE,",%s(%s_alphabet,\n\t\t%s_alphabet_size,\n\t\t%s_inverse_table,",
						lines[i].varName.data,lines[i].varName.data,lines[i].varName.data);
						buf_cpp.AddString( tmp_cpp );
						if (lines[i].isTypeSizeExist)
						{
							snprintf(tmp_cpp
								,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d)",
								lines[i].typeRange.low,lines[i].typeRange.upp,lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp_cpp );
						}
						else
						{
							snprintf(tmp_cpp,
							MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,%d)",
							lines[i].typeRange.isExtendable);
							buf_cpp.AddString( tmp_cpp );

						}

					}
				}
				buf_cpp.AddString( "\n" );
*//*
						}break;
		default:		{
				printf("\n\t(2) Error ! Type is bad!"); exit(-1);
						}
		}
		///.CPP
		//buf_cpp.AddString( tmp_cpp );
		buf_cpp_size = buf_cpp.GetSize()+1;
		buffer_cpp = new char[buf_cpp_size];
		buf_cpp.GetString( buffer_cpp );
		///.HEADER
		buf_size = buf.GetSize()+1;
		buffer = new char[buf_size];
		buf.GetString( buffer );

	}
	/////////////////////////////////////////////////////////////////////////////////////////
	void MakeConstructorBodyAlloc( char *& buffer, unsigned long & buf_size,
					char *& buffer_cpp,
			unsigned long & buf_cpp_size)
	{	///.CPP весь!!!
		if ((type!=e_sequence) && (type!=e_choice) && (type!=e_set)) 		return ;
		VS_AsnCompilerBuffer buf;
		char tmp[MAX_NAME_SIZE]={0};
		snprintf(tmp,MAX_NAME_SIZE,"\t{\n");
		buf.AddString( tmp );
		if ((e_sequence==type) || (e_set==type))
		{
			unsigned int i=0;
			for(i=0;i<num_of_simple;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"\t\t%s[%d].Set(&%s,%d);\n"
					,seq_r
					,i
					,lines[ i ].varName.data
					,lines[ i ].isOptional);
				buf.AddString( tmp );
			}
			if (isExtendable)
			{
				for(i=num_of_simple;i<current_line;i++)
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\t%s[%d].Set(&%s,%d);\n"
						,seq_e
						,i - num_of_simple
						,lines[ i ].varName.data
						,lines[ i ].isOptional);
					buf.AddString( tmp );
				}
			}
		}

		snprintf(tmp,MAX_NAME_SIZE,"\t}\n");
		buf.AddString( tmp );
		///.CPP
		buf_cpp_size = buf.GetSize()+1;
		buffer_cpp = new char[buf_cpp_size];
		buf.GetString( buffer_cpp );
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	void MakeBodyAlloc( char *& buffer, unsigned long & buf_size,
					char *& buffer_cpp,
			unsigned long & buf_cpp_size)
	{
		///.HEADER
		if ((type!=e_sequence) && (type!=e_choice) && (type!=e_set)) 		return ;
		VS_AsnCompilerBuffer buf;
		VS_AsnCompilerBuffer buf_cpp;

		l("111");
		char tmp[MAX_NAME_SIZE]={0};
		snprintf(tmp,MAX_NAME_SIZE,"\n");
		buf.AddString( tmp );
		l("112");
		unsigned int i=0;
		////ALPhabet
		for(i=0;i<current_line;i++)
		{
			lines[ i ].MakeAlphabet( buf, buf_cpp, true, lines[ i ].varName, typeName );
		}
		////ALPhabet end

		if ((e_sequence==type) || (e_set==type))
		{
			snprintf(tmp,MAX_NAME_SIZE,"\n\tstatic const unsigned %s = %d;",seq_b,num_of_simple);
			buf.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn %s[%s];",seq_r,seq_b);
			buf.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\tstatic const unsigned %s = %d;",seq_e,num_of_extension);
			buf.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn %s[%s];",seq_e,seq_x);
			buf.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\n");
			buf.AddString( tmp );
			char tmp1[MAX_NAME_SIZE]={0};
			////ALPhabet
			for(i=0;i<current_line;i++)
			{
				lines[ i ].MakeAlphabet( buf, buf_cpp, true, lines[ i ].varName, typeName );
			}
			////ALPhabet end

			for(i=0;i<num_of_simple;i++)
			{
				lines[ i ].MakeLine( tmp1 );
				snprintf(tmp,MAX_NAME_SIZE,"\n\t%s",tmp1);
				buf.AddString( tmp );
			}
			l("113");
			if (isExtendable)
			{
				snprintf(tmp,MAX_NAME_SIZE,"\n\t \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\");
				printf("\n\t %d ",current_line - num_of_simple);
				for(i=num_of_simple;i<current_line;i++)
				{
					lo("113.3");
					lines[ i ].MakeLine( tmp1 );
					lo("113.4");
					snprintf(tmp,MAX_NAME_SIZE,"\n\t%s",tmp1);
					lo("113.5");
					buf.AddString( tmp );
					lo("113.6");
				}
			}
		}
		l("114");
		buf_size = buf.GetSize()+1;
		buffer = new char[buf_size];
		buf.GetString( buffer );
		/////////////////////////////////
		buf_cpp_size = buf_cpp.GetSize()+1;
		buffer_cpp = new char[buf_cpp_size];
		buf_cpp.GetString( buffer_cpp );

		l("115");
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	void MakeOperatorsAlloc( char *& buffer,
			unsigned long & buf_size,
			char *& buffer_cpp,
			unsigned long & buf_cpp_size)
	{
		if ((type!=e_sequence) && (type!=e_choice) && (type!=e_set)) 		return ;
		VS_AsnCompilerBuffer buf;
		VS_AsnCompilerBuffer buf_cpp;
		char tmp[MAX_NAME_SIZE]={0};
		snprintf(tmp,MAX_NAME_SIZE,"\n");
		buf.AddString( tmp );
		unsigned int i=0;
		if ((e_sequence==type) || (e_set==type))
		{
			///.HEADER
			snprintf(tmp,MAX_NAME_SIZE,"\tvoid operator=(const %s& src);\n",typeName.data);
			buf.AddString( tmp );
			///////////
			snprintf(tmp,MAX_NAME_SIZE,"\tvoid %s::operator=(const %s& src)\n\t{\n\t",typeName.data,typeName.data);
			buf_cpp.AddString( tmp );
			for(i=0;i<current_line;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"\n\t\tO_C(%s);",lines[ i ].varName.data);
				buf_cpp.AddString( tmp );
			}
			buf_cpp.AddString( "\n\t}\n" );

			///.HEADER
            snprintf(tmp,MAX_NAME_SIZE,"\tbool operator==(const %s& src);\n"
				,typeName.data);
			buf.AddString( tmp );
			//////////////


			snprintf(tmp,MAX_NAME_SIZE,"\tbool %s::operator==(const %s& src)\n\t{\n\t\tbool res = src.filled;\n "
				,typeName.data,typeName.data);
			buf_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\tif (res"
				,typeName.data);
			buf_cpp.AddString( tmp );

			for(i=0;i<current_line;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"&& \n\t\t O_T(%s)",lines[ i ].varName.data);
				buf_cpp.AddString( tmp );
			}
			snprintf(tmp,MAX_NAME_SIZE,"\n\t\t) return true;\n\t\t else return false;");
			buf_cpp.AddString( tmp );
			buf_cpp.AddString( "\n\t}\n" );
			buf.AddString( "}\n" );

		}
		if(e_choice==type)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\n \tenum{\n");
			buf.AddString( tmp );
			for(i=0;i<current_line;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"\te_%s",lines[ i ].varName.data);
				buf.AddString( tmp );
				if (i!=(current_line-1)) buf.AddString(",\n");
				else buf.AddString("\n");
			}
			snprintf(tmp,MAX_NAME_SIZE,"\t};\n");
			buf.AddString( tmp );

			/////////////////////////////////////////////////
			///.HEADER
			snprintf(tmp,MAX_NAME_SIZE,"\tbool Decode( VS_Perbuffer &buffer );\n");
			buf.AddString( tmp );


			////////////
			snprintf(tmp,MAX_NAME_SIZE,"\tbool %s::Decode( VS_Perbuffer &buffer )\n\t{\n\t",typeName.data);
			buf_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\tif (!buffer.ChoiceDecode(*this)) return false;\n\t");
			buf_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\tswitch(tag)\n	\t{\n");
			buf_cpp.AddString( tmp );
			for(i=0;i<current_line;i++)
			{
				if (line.type==e_undefined)
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",line.varName.data);
					buf_cpp.AddString( tmp );
					VS_AsnCompilerName ttt;
					VS_AsnCompilerNameTranslator(line.typeName , ttt);
					snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new %s);\n",ttt.data);
					buf_cpp.AddString( tmp );
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",line.varName.data);
					buf_cpp.AddString( tmp );
					snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new ");//%s);\n",string_types[ lines[ i ].type ]);
					buf_cpp.AddString( tmp );
					if (lines[ i ].MakeType( buf_cpp );
					snprintf(tmp,MAX_NAME_SIZE," );\n");
					buf_cpp.AddString( tmp );
				}
			}
			if (isExtendable == true)
			{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tdefault: return buffer.ChoiceMissExtensionObject(*this);");
					buf_cpp.AddString( tmp );
			}
			else
			{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tdefault: return false;");
					buf_cpp.AddString( tmp );
			}
			buf_cpp.AddString( "\n\t\t}\n" );
			buf_cpp.AddString( "\n\t}\n" );
			/////////////////////////////////////////////////
			///.HEADER
			snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid operator=(const %s & src)",typeName.data);
			buf.AddString( tmp );


			/////////////////
			snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid %s::operator=(const %s & src)",typeName.data,typeName.data);
			buf_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n\t{\n\t\tFreeChoice();\n\t\tif (!src.filled) return;\n\t\tswitch(src.tag)\n\t\t{\n");
			buf_cpp.AddString( tmp );
			for(i=0;i<current_line;i++)
			{
				if (lines[ i ].type==e_undefined)
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
					buf_cpp.AddString( tmp );
					VS_AsnCompilerName ttt;
					VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
					snprintf(tmp,MAX_NAME_SIZE,"CopyChoice< %s >(src,*this);\n",ttt.data);
					buf_cpp.AddString( tmp );
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
					buf_cpp.AddString( tmp );
					snprintf(tmp,MAX_NAME_SIZE,"CopyChoice<");// %s >(src,*this);\n",string_types[ lines[ i ].type ]);
					buf_cpp.AddString( tmp );
					lines[ i ].MakeType( buf_cpp );
					buf_cpp.AddString( "  >(src,*this);\n" );
				}
			}
			buf_cpp.AddString( "\t\tdefault:		return;\n\t\t}\n" );
			buf_cpp.AddString( "\n\t\treturn; \n\t}\n" );
			buf.AddString( "\n}\n" );
		}
		buf_size = buf.GetSize()+1;
		buffer = new char[buf_size];
		buf.GetString( buffer );
		/////////////////////////////////////////////////
		buf_cpp_size = buf_cpp.GetSize()+1;
		buffer_cpp = new char[buf_cpp_size];
		buf_cpp.GetString( buffer_cpp );

	}
*/
	//protected:
	//int  res;
	//char tmp[MAX_NAME_SIZE];

};
/////////////////////////////////////////////////////////////////////////////////////////
typedef std::pair<VS_AsnCompilerKey,VS_AsnMetaTypeCompilerContainer>  VS_AsnCompilerContainerPair;
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnCompilerContainer
{	////////////////////////////////////////////////////////////////////////////////////////
	std::map<VS_AsnCompilerKey,VS_AsnMetaTypeCompilerContainer>       class_tree;
	std::map<VS_AsnCompilerKey,VS_AsnMetaTypeCompilerContainer>::iterator class_tree_iterator;
	VS_AsnMetaTypeCompilerContainer                     *current;
	std::vector< VS_AsnCompilerKey >				meta_type_storage;
	unsigned long meta_type_index;
	VS_AsnCompilerKey max_key_value;
	VS_AsnCompilerContainer()
	{
		current = 0;
		max_key_value = 0;
		meta_type_index = 0;
		meta_type_storage.resize( 1 );

		class_tree_iterator = class_tree.begin();
	}
	////////////////////////////////////////////////////////////////////////////////////////
	bool AddMetaType(VS_AsnCompilerName type_name,
						VS_AsnSimpleTypes type ,
						bool isAlphabet ,
						VS_AsnCompilerName& alphabet,
						bool isTypeFilled,
						VS_AsnTypeCompilerContainer current_type)
	{	//world.AddMetaType(*($1) , current_type,isNameIsAlphabet,current_name,isTypeFilled,current_type);
		////////////////Making new type.///////////////////////////////
		printf(" AddMetaType ");
		//VS_AsnCompilerNameCorrector( type_name );
		VS_AsnCompilerNameTranslator( type_name );
		VS_AsnMetaTypeCompilerContainer * meta_type = new VS_AsnMetaTypeCompilerContainer;
		meta_type->Init();
		meta_type->isEmbeded = false;
		meta_type->type = type;
		meta_type->myKey = max_key_value;
		meta_type_storage.insert( meta_type_storage.begin()+ meta_type_index,  max_key_value);
		++meta_type_index;
		if (strncpy(meta_type->typeName.data,type_name.data,MAX_NAME_SIZE)==0)
		{
			printf("\n error in AddMetaType");
			exit(0);
		}
		////////////////////////////////////////////////////////////////

		////////////////ALphabetic ..../////////////////////////////////
		if (isAlphabet) meta_type->alphabet = alphabet;
		meta_type->isAlphabetExist = isAlphabet;
		printf("\n\t ISALPH:%d TYPE:%s\n",isAlphabet,type_name.data);
		////////////////////////////////////////////////////////////////

		////////////////Types ........./////////////////////////////////
		if (isTypeFilled)
		{
			meta_type->isArrayExist    = current_type.isArrayExist;
			meta_type->arrayRange	   = current_type.arrayRange;
			meta_type->arrayType	   = current_type.arrayType;
			meta_type->isAlphabetExist = current_type.isAlphabetExist;
			meta_type->isArraySizeExist= current_type.isArraySizeExist;
			meta_type->isTypeSizeExist = current_type.isTypeSizeExist;
			meta_type->typeRange	   = current_type.typeRange;
			meta_type->isTypeSizeExist = current_type.isTypeSizeExist;

			if (current_type.type==e_undefined)
			{
				//meta_type->type = e_undefined;
				VS_AsnCompilerNameTranslator( current_type.typeName );
				//if (strncpy(meta_type->typeName.data,current_type.typeName.data,MAX_NAME_SIZE)==0)
				//{
				//	printf("\n error in AddMetaType");
				//	exit(0);
				//}

				if (strncpy(meta_type->simple_meta_name.data, current_type.typeName.data,MAX_NAME_SIZE)==0)
				{
					printf("\n error in AddMetaType");
					exit(0);
				}
			}
		}
		////////////////////////////////////////////////////////////////

		VS_AsnCompilerContainerPair meta = VS_AsnCompilerContainerPair( max_key_value , *meta_type);
		class_tree.insert( meta );
		////////////////////////////////////////////////////////////////

		////////////////Redirecting..../////////////////////////////////
		current = &(class_tree[ max_key_value ]);
		printf(" AddMetaType current: %p",current);
		max_key_value++;
		////////////////////////////////////////////////////////////////
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	bool AddEmbededType(VS_AsnCompilerName variable_name,
						VS_AsnSimpleTypes type,
						bool is_optional)
	{
		////////////////There is a the work in current type////////////////////
		///add info to line.
		///var_name
		current->lines[ current->current_line ].Init();
		variable_name = VS_AsnCompilerVarNameCorrector( variable_name );
		printf("\n\t AddEmbededType  press any");
		////////
		unsigned int koeff=3;
		printf("\n\n\t LINE: %lu  SIZE:%lu REAL SIZE : %lu",current->current_line,
			   current->line_size,(unsigned long)current->lines.size());
		if ( current->current_line + 2 > current->lines.size())
		{
			printf("\n\n\t RESIZING!!!!\n\n");
			current->lines.resize( current->line_size + koeff );
			current->line_size += koeff;
		}
		////////
		//current->lines.resize( 10 + current->line_size );
		//current->line_size += 10;
		if (strncpy(current->lines[ current->current_line ].varName.data ,
					(char *)( variable_name.data ),
					MAX_NAME_SIZE)==0)	{printf("\t\t\terror in AddEmbededType");	return false;}
		///typename
		VS_AsnCompilerEmbededNameTranslator( variable_name , current->typeName );
		if (strncpy((char *)(current->lines[ current->current_line ].typeName.data) ,
					(char *)( variable_name.data ),
					MAX_NAME_SIZE)==0)	{printf("\t\t\terror in AddEmbededType"); return false;}
		///type
		current->lines[ current->current_line ].type = type;
		///is optional
		current->lines[ current->current_line ].isOptional = is_optional;
		current->lines[ current->current_line ].isEmbeded = true;
		current->lines[ current->current_line ].myKey = max_key_value;
		//if (is_optional && (!current->isExtendable)) (current->num_of_options)++;

		///extend
		if (current->isExtendable) (current->num_of_extension)++;
		else (current->num_of_simple)++;
		current->current_line++;
		////////////////End of work in current type////////////////////

		////////////////Making new type.///////////////////////////////
		VS_AsnMetaTypeCompilerContainer * embeded_type = new VS_AsnMetaTypeCompilerContainer;
		embeded_type->Init();
		///add info
		///fathers name
		if (strncpy( (char *)(embeded_type->fathersName.data),
					 (char *)(current->typeName.data),
					 MAX_NAME_SIZE)==0) {printf("\t\t\terror in AddEmbededType"); return false;}
		///my name
		if (strncpy( (char *)( embeded_type->typeName.data),
					 (char *)( variable_name.data ),
					 MAX_NAME_SIZE)==0) {printf("\t\t\terror in AddEmbededType"); return false;}


		///fathers key
		embeded_type->fathersKey = current->myKey;
		///type
		embeded_type->type = type;
		///key.
		embeded_type->myKey = max_key_value;
		VS_AsnCompilerContainerPair embeded = VS_AsnCompilerContainerPair( max_key_value , *embeded_type  );
		class_tree.insert( embeded );
		////////////////End of making new type./////////////////////////

		////////////////Redirecting..../////////////////////////////////
		current = &class_tree[ max_key_value ];
		//if (max_key_value==8) embeded_type->Show();
		max_key_value++;

		////////////////////////////////////////////////////////////////
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	bool AddLinePrefix(	VS_AsnCompilerName& variable_name,
						bool is_embeded,
						bool is_array_exist,
						VS_AsnSimpleTypes array_type,
						VS_AsnCompilerRange range  )
	{
		unsigned int koeff=2;
		printf("\n\n\t LINE: %lu  SIZE:%lu REAL SIZE : %lu",current->current_line,
			   current->line_size,(unsigned long)current->lines.size());
		if ( current->current_line + 2 > current->lines.size())
		{
			printf("\n\n\t RESIZING!!!!\n\n");
			current->lines.resize( current->line_size * koeff );
			current->line_size *= koeff;
		}
		current->lines[ current->current_line ].Init();
		VS_AsnCompilerName temp = VS_AsnCompilerVarNameCorrector(variable_name);
		if (strncpy((char *)(current->lines[ current->current_line ].varName.data) ,
					(char *)( temp.data ),
					MAX_NAME_SIZE)==0)	return false;
		current->lines[ current->current_line ].isEmbeded = is_embeded;
		if (is_embeded)
			return true;/// Остальное в постфиксе.
		if (is_array_exist)
		{	current->lines[ current->current_line ].isArrayExist = true;
			current->lines[ current->current_line ].arrayType  = array_type;
			current->lines[ current->current_line ].arrayRange = range;
			if ((range.low == range.upp) && (0== range.upp))
				current->lines[ current->current_line ].isArraySizeExist = false;
			else current->lines[ current->current_line ].isArraySizeExist = true;
			return true;
		}
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	bool AddLinePostfixCrypto(VS_AsnCompilerName type_name,
						bool is_type_simple,
						VS_AsnSimpleTypes type,
						bool is_size_exist,
						VS_AsnCompilerRange range,
						bool is_optional)
	{
		current->lines[ current->current_line ].isAlphabetExist = false;
		current->lines[ current->current_line ].type = type;
			//VS_AsnCompilerName temp = VS_AsnCompilerVarNameCorrector(type_name);
			if (strncpy((char *)(current->lines[ current->current_line ].typeName.data) ,
				(char *)( type_name.data ),
				MAX_NAME_SIZE)==0)	return false;
		if (is_size_exist)
		current->lines[ current->current_line ].typeRange = range;
		current->lines[ current->current_line ].isOptional = is_optional;
		current->lines[ current->current_line ].isTypeSizeExist = is_size_exist;
		if (is_optional && (!current->isExtendable)) (current->num_of_options)++;// (current->num_of_options)++;
		if (current->isExtendable) (current->num_of_extension)++;
		else (current->num_of_simple)++;
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	bool AddLinePostfix(VS_AsnCompilerName type_name,
						bool is_type_simple,
						VS_AsnSimpleTypes type,
						bool is_size_exist,
						VS_AsnCompilerRange range,
						bool is_optional)
	{
		current->lines[ current->current_line ].isAlphabetExist = false;
		if (is_type_simple) current->lines[ current->current_line ].type = type;
		else
		{
			//VS_AsnCompilerName temp = VS_AsnCompilerVarNameCorrector(type_name);
			if (strncpy((char *)(current->lines[ current->current_line ].typeName.data) ,
				(char *)( type_name.data ),
				MAX_NAME_SIZE)==0)	return false;
			current->lines[ current->current_line ].type = e_undefined;
		}
		if (is_size_exist)
		current->lines[ current->current_line ].typeRange = range;
		current->lines[ current->current_line ].isOptional = is_optional;
		current->lines[ current->current_line ].isTypeSizeExist = is_size_exist;
		if (is_optional && (!current->isExtendable)) (current->num_of_options)++;// (current->num_of_options)++;
		if (current->isExtendable) (current->num_of_extension)++;
		else (current->num_of_simple)++;
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	bool AddAlphabetLinePostfix(VS_AsnCompilerName& alphabet,
						bool is_type_simple,
						VS_AsnSimpleTypes type,
						bool is_size_exist,
						VS_AsnCompilerRange range,
						bool is_optional
						)
	{

		if (is_type_simple) current->lines[ current->current_line ].type = type;
		else
		{
			printf("\n\t ERROR : AddAlphabetLinePostfix type = e_undef!!!");
			exit(0);
		}
		current->lines[ current->current_line ].alphabet = alphabet;
		current->lines[ current->current_line ].isAlphabetExist = true;
		if (is_size_exist)
		current->lines[ current->current_line ].typeRange = range;
		current->lines[ current->current_line ].isOptional = is_optional;
		current->lines[ current->current_line ].isTypeSizeExist = is_size_exist;
		if (is_optional && (!current->isExtendable)) (current->num_of_options)++;// (current->num_of_options)++;
		if (current->isExtendable) (current->num_of_extension)++;
		else (current->num_of_simple)++;
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	void AddBadPostfix()
	{
		current->lines[ current->current_line ].SetBad();
		current->SetBad();
	}
	////////////////////////////////////////////////////////////////////////////////////////
	void AddDots()
	{	current->isExtendable = true;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	void EmbededOut()
	{
		VS_AsnCompilerKey fathers_key = current->fathersKey;
		current = &(class_tree[ fathers_key ]);

	}
	void LastEmbeddedIsOptional(bool isOptional)
	{
		current->lines[ current->current_line - 1 ].isOptional = isOptional;
		if (isOptional && (!current->isExtendable)) (current->num_of_options)++;// (current->num_of_options)++;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	void MetaOut()
	{
		current = 0;
	}
	////////////////////////////////////////////////////////////////////////////////////////
	void NextLine()
	{
		//current->line_size+=2;
		//current->lines.resi( current->line_size );
		current->current_line++;
	}
	////////////////////////////////////////////////////////////////////////////////////////

};




