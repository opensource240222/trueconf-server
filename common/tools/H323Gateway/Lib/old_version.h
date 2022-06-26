/*
	void MakeSize(VS_AsnCompilerBuffer &buffer)
	{
			//buffer.AddString( "*" );
			char tmp[MAX_NAME_SIZE]={0};
			if (isArrayExist)
			{
				if (isArraySizeExist)
				{
					snprintf(tmp,MAX_NAME_SIZE,"%s< ", string_types[17]);
					buffer.AddString( tmp );
					MakeType( buffer );
					snprintf(tmp,MAX_NAME_SIZE," ,%d,%lu,VS_Asn::FixedConstraint,%d> \t",
						arrayRange.low,
						arrayRange.upp,
						arrayRange.isExtendable);
					buffer.AddString( tmp );
					return;
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"%s< ", string_types[16]);
					buffer.AddString( tmp );
					MakeType( buffer );
					snprintf(tmp,MAX_NAME_SIZE,"> \t");
					buffer.AddString( tmp );
					return;
				}
			}
			return;
	}
*/

		/*
	/////////////////////////////////////////////////////////////////////////////////////////
	void MakeTypeForChoice(VS_AsnCompilerBuffer &buffer)
	{
		char tmp[MAX_NAME_SIZE]={0};

		if ((type==e_undefined) || (isEmbeded==true))
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
			else
			{
				snprintf(tmp,MAX_NAME_SIZE,"%s\t",typeName.data );
				buffer.AddString( tmp );
				return;
			}
		}
		else
		{
			if (isTypeSizeExist)
			{
				switch(type)
				{
				case e_integer:
					 {
						 snprintf(tmp,MAX_NAME_SIZE,"TemplInteger<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					 }break;
				case e_bitstring:
					 {
						 snprintf(tmp,MAX_NAME_SIZE,"TemplBitString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;

					 }break;
				case e_octstring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplOctetString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;

					}break;
				case e_numstring:
					{
						printf("\n\t ERROR : Numeric string have not released yet!\nAsk Alex.");
						exit(0);
					}break;
				case e_ia5string:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplIA5String<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					}break;
				case e_pristring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplPrintableString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					}break;
				case e_genstring:
					{
						printf("\n\t ERROR: General string not released yet!\n Ask Alex.");
						exit(0);
					}break;
				case e_bmpstring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplBMPString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					}break;
				default:
					{
						printf("\n\t ERROR bad sized type!\n");
						//printf("\n\t varname : %s type:%d",varName.data, type);
						exit(0);
					}
				}
			}
			else
			{
				if (isAlphabetExist)
				{

					if (type == e_ia5string)
					{
						snprintf(tmp,
							MAX_NAME_SIZE,
							"TemplIA5String<0,INT_MAX,VS_Asn::Unconstrained,false> ");
						buffer.AddString( tmp );
						return ;
					}

					snprintf(tmp,MAX_NAME_SIZE," VS_AsnRestrictedString ");
					buffer.AddString( tmp );
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"%s ",string_types[ type ]	);
					buffer.AddString( tmp );
				}
				return;
			}
		}

	}
	/////////////////////////////////////////////////////////////////////////////////////////
*/
/*	void MakeType(VS_AsnCompilerBuffer &buffer)
	{
		char tmp[MAX_NAME_SIZE]={0};

		if ((type==e_undefined) || (isEmbeded==true))
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
			else
			{
				snprintf(tmp,MAX_NAME_SIZE,"%s\t",typeName.data );
				buffer.AddString( tmp );
				return;
			}
		}
		else
		{
			if (isTypeSizeExist)
			{
				switch(type)
				{
				case e_integer:
					 {
						 snprintf(tmp,MAX_NAME_SIZE,"TemplInteger<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					 }break;
				case e_bitstring:
					 {
						 snprintf(tmp,MAX_NAME_SIZE,"TemplBitString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;

					 }break;
				case e_octstring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplOctetString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;

					}break;
				case e_numstring:
					{
						printf("\n\t ERROR : Numeric string have not released yet!\nAsk Alex.");
						exit(0);
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
						 snprintf(tmp,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );

						 return;
					}break;
				case e_pristring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplPrintableString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					}break;
				case e_genstring:
					{
						printf("\n\t ERROR: General string not released yet!\n Ask Alex.");
						exit(0);
					}break;
				case e_bmpstring:
					{
						 snprintf(tmp,MAX_NAME_SIZE,"TemplBMPString<%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );
						 return;
					}break;
				default:
					{
						printf("\n\t ERROR bad sized type!\n");
						//printf("\n\t varname : %s type:%d",varName.data, type);
						exit(0);
					}
				}
			}
			else
			{
				if (isAlphabetExist)
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
						 snprintf(tmp,MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d> ",
							 typeRange.low,typeRange.upp,typeRange.isExtendable);
						 buffer.AddString( tmp );

					////snprintf(tmp,MAX_NAME_SIZE," VS_AsnRestrictedString ");
					////buffer.AddString( tmp );
				}
				else
				{
					snprintf(tmp,MAX_NAME_SIZE,"%s ",string_types[ type ]	);
					buffer.AddString( tmp );
				}
				return;
			}
		}

	}
*/

/*
	void MakeAlphabet( VS_AsnCompilerBuffer &buffer, VS_AsnCompilerBuffer &buffer_cpp
		,bool is_varname , VS_AsnCompilerName &name, VS_AsnCompilerName &meta_name)
	{
		if (!isAlphabetExist) return;
		char tmp[MAX_NAME_SIZE]={0};
		//if (is_varname)
		//{
			///.HEADER
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char   %s_alphabet[];\n",name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned		%s_alphabet_size[];\n",name.data );
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char   %s_inverse_table[];\n",name.data);
			buffer.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t static const bool      %s_flag_set_table;\n",name.data);
			buffer.AddString( tmp );
			/////////

			///.CPP

			snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char %s::%s_alphabet[]=\n\t{",meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			unsigned i = 0;
			unsigned limit = strlen( alphabet.data );
			for(i=0;i<limit;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"'%c'",alphabet.data[i]);
				buffer_cpp.AddString( tmp );
				if (i+1 != limit)
					buffer_cpp.AddString( "," );
			}
			buffer_cpp.AddString( " };\n" );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned  %s::%s_alphabet_size[]=sizeof(%s::%s_alphabet[]);\n"
				,meta_name.data,name.data,meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\t static unsigned char  %s::%s_inverse_table[256]={0};\n",meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			//
			snprintf(tmp,MAX_NAME_SIZE,"\t static const bool      %s_flag_set_table = \n",name.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t VS_AsnRestrictedString::MakeInverseCodeTable(\n");
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_inverse_table,\n",meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_alphabet,\n",meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\t\t %s::%s_alphabet_size );\n",meta_name.data,name.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
			buffer_cpp.AddString( tmp );
		///}
		///else
		//{
		//}
	}
	//protected:
*/