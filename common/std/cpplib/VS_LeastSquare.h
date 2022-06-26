/**
 **************************************************************************
 * \file VS_LeastSquare.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief least square error procedure class
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 21.07.03
 *
 * $Revision: 1 $
 *
 * $History: VS_LeastSquare.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 ****************************************************************************/
#ifndef VS_LEASTSQUARE_H
#define VS_LEASTSQUARE_H


/**
 **************************************************************************
 * \brief Collect piars of values and try represent it as linear function
 * by least square error procedure
 ****************************************************************************/
class VS_LeastSquare
{
public:
	VS_LeastSquare() :m_count(0), m_x(0), m_y(0), m_val(0), m_yCount(0){}
	~VS_LeastSquare(){
		if (m_x) delete[] m_x;
		if (m_y) delete[] m_y;
		if (m_yCount) delete[] m_yCount;
	}
	void Init(int Count) {
		int i;
		if (Count<20) Count = 20;
		m_count = Count;
		if (m_x) delete[] m_x;
		if (m_y) delete[] m_y;
		if (m_yCount) delete[] m_yCount;
		m_x = new double[m_count];
		m_y = new double[m_count];
		m_yCount = new double[m_count];
		for (i = 0 ; i<m_count; i++){
			m_x[i]=m_y[i]=m_yCount[i] = 0.;
		}
		m_val = 0;
	}
	void Fill(double x, double y)	{
		int i;
		for (i = 0; i<m_val; i++) {
			if (m_x[i]==x) {
				m_y[i] *= m_yCount[i];
				m_yCount[i] += 1.;
				m_y[i] = (m_y[i] + y)/m_yCount[i];
				return;
			}
		}
		if (m_val<m_count) {
			m_x[m_val] = x;
			m_y[m_val] = y;
			m_yCount[m_val] = 1.;
			m_val++;
		}
		else {// interchange
			for (i = 0; i< (m_count-1); i++){
				m_x[i] = m_x[i+1];
				m_y[i] = m_y[i+1];
				m_yCount[i] = m_yCount[i+1];
			}
			m_x[i] = x;
			m_y[i] = y;
			m_yCount[i] = 1.;
		}
	}
	bool GetParam(double *a0, double *a1) // P(x) = a0 + a1*x;
	{
		*a0 = 0.;
		*a1 = 0.;
		if (m_val<20) 	return false;
		double A0 , A1, A2, B, D;
		int i;

		A0 = A1 = A2 =  B = D = 0.;
		for (i = 0; i<m_val; i++) {
			A0+=1.;
			A1+=(m_x[i]);
			A2+=(m_x[i]*m_x[i]);
			B +=(m_y[i]);
			D +=(m_y[i]*m_x[i]);
		}
		if (A1!=0. && (A0*A2-A1*A1)!=0.) {
			*a0 = (B*A2-D*A1)/(A0*A2-A1*A1);
			*a1 = (B-A0**a0)/A1;
			return true;
		}
		else
			return false;
	}
	bool Predict(double x, double &y) {
		double a0, a1;
		if (GetParam(&a0, &a1)) {
			y = a0+a1*x;
			return true;
		}
		return false;
	}

	double		*m_x, *m_y,*m_yCount;
	int			m_count;		// num of total val
	int			m_val;			// num of filled val
};

#endif
