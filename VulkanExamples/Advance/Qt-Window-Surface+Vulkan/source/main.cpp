/*

*


*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#include "Headers.h"
#include "VulkanApplication.h"
//#include "../QtSource/rasterwindow.h"
#include <QtGui>
#include <QMainWindow>

//HWND m_winId;

std::vector<const char *> instanceExtensionNames = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
};

std::vector<const char *> layerNames = {
	"VK_LAYER_LUNARG_standard_validation"
};

std::vector<const char *> deviceExtensionNames = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//RasterWindow* window = NULL;
#include <QtGui>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>

class  myMainWindow :public QMainWindow
{
public:
	myMainWindow() :QMainWindow()
	{
		//QPalette* palette = new QPalette();
		//palette->setBrush(QPalette::Background, *(new QBrush(*(new QPixmap("b.jpg")))));
		//setPalette(*palette);

		//setWindowFlags(Qt::FramelessWindowHint);
		QWidget *centralWidget = new QWidget(this);
		//QGridLayout *layout = new QGridLayout();
		QHBoxLayout *layout = new QHBoxLayout();

		centralWidget->setLayout(layout);


		//layout->addWidget(new QPushButton("Button  1"), 0, 0);
		//layout->addWidget(new QPushButton("Button  2"), 0, 1);
		//layout->addWidget(new QPushButton("Button  3"), 0, 2);
		VulkanApplication* appObj = VulkanApplication::GetInstance();
		appObj->initialize();
		appObj->prepare();

		//vulkan_window = new VulkanRenderer(appObj, appObj->deviceObj);
		//vulkan_window = appObj->rendererObj;

		//MainWindow* w = new MainWindow();
		layout->addWidget(QWidget::createWindowContainer(appObj->rendererObj));
//		w.show();


		setCentralWidget(centralWidget);
	};
	~myMainWindow() {};

};

int main(int argc, char **argv)
{	
	QApplication app(argc, argv);
	myMainWindow *window = new myMainWindow();


	window->resize(512, 512);
	window->show();
	//VulkanApplication* appObj = VulkanApplication::GetInstance();
	//appObj->initialize();
	//appObj->prepare();
	//MainWindow w;
	//w.show();
	app.exec();
	return 0;
}
