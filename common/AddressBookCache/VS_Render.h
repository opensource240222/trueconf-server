#ifndef _VS_RENDER_H_
#define _VS_RENDER_H_

class QWidget;
class QString;
class QRectF;
class QPoint;

class VS_Render
{
public:
	virtual QObject * getQObject() = 0;

	//virtual void addBottomPanel(QWidget * aPanel) = 0;
	//virtual void addBottomPanelMenu(QWidget * aMenu) = 0;
	//virtual void setBottomPanelVisible(bool aVisible) = 0;
	virtual void setNoVideoVisible(bool aVisible) = 0;
	virtual void setNoSoundVisible(bool aVisible) = 0;
	virtual void setDisplayLabel(const QString & aStr) = 0;

	virtual void setGeometry(const QRectF &aGeometry) = 0;
	virtual QRectF geometry() const = 0;
	virtual void raise(int orderNumber = 0) = 0;
	virtual void redraw() = 0;

	virtual void UpdateFrame(unsigned char *rawData, unsigned long bits, unsigned long width, unsigned long height,int stereo = 0,
					 unsigned long x = 0, unsigned long y = 0, unsigned long rectWidth = 0, unsigned long rectHeight = 0) = 0;
	//signals
	/*virtual void onHover(bool) = 0;
	virtual void rightClick(const QPoint &aPos) = 0;
	virtual void volumeUp() = 0;
	virtual void volumeDown() = 0;*/

	virtual void setFocused() = 0;
};

#endif