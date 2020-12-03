
#include <QDialog>
#include <QDebug>
#pragma execution_character_set("utf-8")

namespace Ui
{
	class EventDialog;
}

class Event : public QDialog
{
	Q_OBJECT
public:
	Event(QWidget* parent = nullptr);

private:
	Ui::EventDialog* ui;

public slots:
	void PressOk();
};

