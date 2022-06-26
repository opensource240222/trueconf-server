#pragma once

#include <cmath>
#include <cstdlib>

/****************************************************************************
 * \brief Count standart deviation of sequence data
 ****************************************************************************/
template <typename T>
class VS_TimeDeviation
{
	T		*m_values;			///< pointer to analized values
	int		m_MaxCount;			///< max count of analized values
	int		m_CurrCount;		///< current number of analized values
	double	m_MaxDiff;			///< max differense error
	double	m_Deviation;		///< deviation of data
	double	m_AverData;			///< average of input data
	bool	m_IsValid;			///< true if Init() called
	bool	m_NewSample;		///< true if coef became old

	/// Predict 1-st order polinom for data by LSE method
	bool Predict(double &a0, double &a1);
	bool Calculate();
public:
	/// Constructor
	VS_TimeDeviation();
	/// Destructor
	~VS_TimeDeviation();
	/// Add (time) value to analised buffer
	void Snap(T val);
	/// Get square deviation
	bool GetDeviation(T& deviation);
	/// Get average of data
	bool GetAverage(T& aver);
	/// Get max diff between predicted line and value
	bool GetMax(T& maxd);
	/// Get average time value
	bool GetPredicted(double number, double& val);
	/// Get average time value for last value
	bool GetPredictedFromLast(double offset, double& val);
	/// Initialization
	void Init(int MaxCount);
	/// Reset counter for new accumulation of data
	void Clear();
};

template <typename T>
VS_TimeDeviation<T>::VS_TimeDeviation()
{
	m_values = 0;
	m_MaxCount = 0;
	m_CurrCount = 0;
	m_MaxDiff = 0;
	m_Deviation = 0;
	m_AverData = 0;
	m_IsValid = false;
	m_NewSample = true;
}

/**
 **************************************************************************
 ****************************************************************************/
template <typename T>
VS_TimeDeviation<T>::~VS_TimeDeviation()
{
	if (m_values) free(m_values);
}

/**
 **************************************************************************
 * \param	MaxCount [in] max number of analised samples
 ****************************************************************************/
template <typename T>
void VS_TimeDeviation<T>::Init(int MaxCount)
{
	if (m_values) free(m_values);
	m_MaxCount = MaxCount;
	m_values = (T*)malloc(MaxCount * sizeof(T));
	m_CurrCount = 0;
	m_IsValid = true;
}

/**
 **************************************************************************
 ****************************************************************************/
template <typename T>
void VS_TimeDeviation<T>::Clear()
{
	m_CurrCount = 0;
	m_MaxDiff = 0;
	m_Deviation = 0;
	m_AverData = 0;
}

/**
 **************************************************************************
 * \return -1 if not enouth accumulated  data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::Calculate()
{
	if (!m_IsValid) return false;
	if (m_NewSample == false) return true;
	int count = m_CurrCount < m_MaxCount ? m_CurrCount : m_MaxCount;
	double D = 0.;
	double a0 = 0., a1 = 0., d = 0.;
	int i = 0;
	if (!Predict(a0, a1)) return false;
	double maxd = 0;
	double aver = 0;
	for (i = m_CurrCount - count; i < m_CurrCount; i++) {
		double val = (double)m_values[i%m_MaxCount];
		aver += val;
		d = a0 + a1 * i - val;
		d *= d;
		D += d;
		if (maxd < d) maxd = d;
	}
	m_AverData = aver / count;
	m_MaxDiff = sqrt(maxd);
	m_Deviation = sqrt(D / (double)count);
	m_NewSample = false;
	return true;
}

/**
 **************************************************************************
 * \return false if not enouth accumulated  data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::GetPredicted(double number, double& val)
{
	if (!m_IsValid) return false;
	double a0 = 0., a1 = 0.;
	if (!Predict(a0, a1))
		return false;
	val = a0 + a1 * number;
	return true;
}

/**
**************************************************************************
* \return false if not enouth accumulated  data
****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::GetPredictedFromLast(double offset, double& val)
{
	return GetPredicted(m_CurrCount + offset - 1.0, val);
}

/**
**************************************************************************
 * \param		aver [out] average of data
 * \return false if not enough data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::GetAverage(T& aver)
{
	if (!Calculate())
		return false;
	aver = m_AverData > 0 ? static_cast<T>(m_AverData + 0.5) : static_cast<T>(m_AverData - 0.5);
	return true;
}

/**
**************************************************************************
 * \param		dmax [out] max difference error
 * \return false if not enough data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::GetMax(T& dmax)
{
	if (!Calculate())
		return false;
	dmax = static_cast<T>(m_MaxDiff + 0.5);
	return true;
}

/**
**************************************************************************
 * \param		dmax [out] max difference error
 * \return false if not enough data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::GetDeviation(T& deviation)
{
	if (!Calculate())
		return false;
	deviation = static_cast<T>(m_Deviation + 0.5);
	return true;
}

/**
 **************************************************************************
 * \param		a0 [in/out] a0 coef
 * \param		a1 [in/out] a1 coef
 * \return false if not enouth accumulated  data
 ****************************************************************************/
template <typename T>
bool VS_TimeDeviation<T>::Predict(double &a0, double &a1)
{
	a0 = 0.; a1 = 0.;
	if (m_CurrCount < m_MaxCount / 50 || (m_CurrCount < 5 && m_MaxCount>5))
		return false; // min 2 % of data or 5 count!
	double A0, A1, A2, B, D;

	A0 = A1 = A2 = B = D = 0.;
	int count = m_CurrCount < m_MaxCount ? m_CurrCount : m_MaxCount;
	for (int i = m_CurrCount - count; i < m_CurrCount; i++) {
		double ind = (double)i;
		double val = (double)m_values[i%m_MaxCount];
		A0 += 1.;
		A1 += ind;
		A2 += ind * ind;
		B += val;
		D += val * ind;
	}
	if (A1 != 0. && (A0*A2 - A1 * A1) != 0.) {
		a0 = (B*A2 - D * A1) / (A0*A2 - A1 * A1);
		a1 = (B - A0 * a0) / A1;
		return true;
	}
	else return false;
}

/**
 **************************************************************************
 * \param		val [in] new current time value
 ****************************************************************************/
template <typename T>
void VS_TimeDeviation<T>::Snap(T val)
{
	m_values[m_CurrCount%m_MaxCount] = val;
	m_CurrCount++;
	m_NewSample = true;
}
