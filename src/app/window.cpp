#include <QtWidgets>
#include <app/tabs/data.hpp>
#include <app/tabs/media.hpp>
#include <app/tabs/settings.hpp>
#include <app/window.hpp>
MainWindow::MainWindow()
{
    this->config = Config::get_instance();
    this->setWindowOpacity(this->config->get_brightness() / 255.0);

    this->theme = Theme::get_instance();
    this->theme->set_mode(this->config->get_dark_mode());
    this->theme->set_color(this->config->get_color());

    QWidget *widget = new QWidget(this);
    this->layout = new QStackedLayout(widget);

    this->layout->addWidget(this->window_widget());

    setCentralWidget(widget);
   
    


}


QWidget *MainWindow::window_widget()
{
    QWidget *widget = new QWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(this->tabs_widget());
    if(this->config->get_controls_visible())
    layout->addWidget(this->controls_widget());
    else{
        this->config->set_volume(100);
        MainWindow::update_system_volume(100);
    }

    setCentralWidget(widget);
    this->theme->update();
    return widget;
}

QTabWidget *MainWindow::tabs_widget()
{
    QTabWidget *widget = new QTabWidget(this);
    widget->setTabPosition(QTabWidget::TabPosition::West);
    widget->tabBar()->setIconSize(Theme::icon_48);

    widget->addTab(new OpenAutoTab(this), QString());
    this->theme->add_tab_icon("directions_car", 0, Qt::Orientation::Vertical);

    widget->addTab(new MediaTab(this), QString());
    this->theme->add_tab_icon("play_circle_outline", 1, Qt::Orientation::Vertical);

    widget->addTab(new DataTab(this), QString());
    this->theme->add_tab_icon("speed", 2, Qt::Orientation::Vertical);

    widget->addTab(new SettingsTab(this), "");
    this->theme->add_tab_icon("tune", 3, Qt::Orientation::Vertical);

    connect(this->config, &Config::brightness_changed, [this, widget](int position) {
        this->setWindowOpacity(position / 255.0);
        if (widget->currentIndex() == 0) emit set_openauto_state(position);
    });
    connect(this->theme, &Theme::icons_updated,
            [widget](QList<tab_icon_t> &tab_icons, QList<button_icon_t> &button_icons) {
                for (auto &icon : tab_icons) widget->tabBar()->setTabIcon(icon.first, icon.second);
                for (auto &icon : button_icons) icon.first->setIcon(icon.second);
            });
    connect(widget, &QTabWidget::currentChanged, [this](int index) {
        emit set_openauto_state((index == 0) ? (windowOpacity() * 255) : 0);
        emit set_data_state(index == 2);
    });

    return widget;
}

QWidget *MainWindow::controls_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addStretch(5);
    layout->addWidget(this->volume_widget(), 3);

    return widget;
}

QWidget *MainWindow::volume_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QSlider *slider = new QSlider(Qt::Orientation::Horizontal, widget);
    slider->setRange(0, 100);
    slider->setSliderPosition(this->config->get_volume());
    update_system_volume(slider->sliderPosition());
    connect(slider, &QSlider::valueChanged, [config = this->config](int position) {
        config->set_volume(position);
        MainWindow::update_system_volume(position);
    });

    QPushButton *lower_button = new QPushButton(widget);
    lower_button->setFlat(true);
    lower_button->setIconSize(Theme::icon_32);
    this->theme->add_button_icon("volume_mute", lower_button);
    connect(lower_button, &QPushButton::clicked, [slider]() {
        int position = slider->sliderPosition() - 10;
        slider->setSliderPosition(position);
    });

    QPushButton *raise_button = new QPushButton(widget);
    raise_button->setFlat(true);
    raise_button->setIconSize(Theme::icon_32);
    this->theme->add_button_icon("volume_up", raise_button);
    connect(raise_button, &QPushButton::clicked, [slider]() {
        int position = slider->sliderPosition() + 10;
        slider->setSliderPosition(position);
    });

    layout->addWidget(lower_button);
    layout->addWidget(slider, 4);
    layout->addWidget(raise_button);

    return widget;
}

void MainWindow::update_system_volume(int position)
{
    QProcess *lProc = new QProcess();
    std::string command = "amixer set Master " + std::to_string(position) + "% --quiet";
    lProc->start(QString(command.c_str()));
    lProc->waitForFinished();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit is_ready();
}

void MainWindow::add_widget(QWidget *widget)
{
    this->layout->addWidget(widget);
    this->layout->setCurrentIndex(1);
}

void MainWindow::remove_widget(QWidget *widget)
{
    this->layout->removeWidget(widget);
    this->layout->setCurrentIndex(0);
}
