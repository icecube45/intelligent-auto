#include <QApplication>
#include <QDir>
#include <QTimer>

#include <app/config.hpp>

Config::Config()
    : QObject(qApp),
      openauto_config(std::make_shared<f1x::openauto::autoapp::configuration::Configuration>()),
      ia_config(QSettings::IniFormat, QSettings::UserScope, "ia")
{
    this->volume = this->ia_config.value("volume", 50).toInt();
    this->dark_mode = this->ia_config.value("dark_mode", false).toBool();
    this->brightness = this->ia_config.value("brightness", 255).toInt();
    this->si_units = this->ia_config.value("si_units", false).toBool();
    this->color = this->ia_config.value("color", "blue").toString();
    this->bluetooth_device = this->ia_config.value("Bluetooth/device", QString()).toString();
    this->radio_station = this->ia_config.value("Radio/station", 98.0).toDouble();
    this->radio_muted = this->ia_config.value("Radio/muted", true).toBool();
    this->controls_visible = this->ia_config.value("Volume Visible", false).toBool();
    this->media_home = this->ia_config.value("media_home", QDir().absolutePath()).toString();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() { this->save(); });
    timer->start(10000);
}

void Config::save()
{
    if (this->volume != this->ia_config.value("volume", 50).toInt())
        this->ia_config.setValue("volume", this->volume);
    if (this->dark_mode != this->ia_config.value("dark_mode", false).toBool())
        this->ia_config.setValue("dark_mode", this->dark_mode);
    if (this->brightness != this->ia_config.value("brightness", 255).toInt())
        this->ia_config.setValue("brightness", this->brightness);
    if (this->si_units != this->ia_config.value("si_units", false).toBool())
        this->ia_config.setValue("si_units", this->si_units);
    if (this->color != this->ia_config.value("color", "blue").toString())
        this->ia_config.setValue("color", this->color);
    if (this->bluetooth_device != this->ia_config.value("Bluetooth/device", QString()).toString())
        this->ia_config.setValue("Bluetooth/device", this->bluetooth_device);
    if (this->radio_station != this->ia_config.value("Radio/station", 98.0).toDouble())
        this->ia_config.setValue("Radio/station", this->radio_station);
    if (this->radio_muted != this->ia_config.value("Radio/muted", true).toBool())
        this->ia_config.setValue("Radio/muted", this->radio_muted);
    if (this->controls_visible != this->ia_config.value("Controls Visible", true).toBool())
            this->ia_config.setValue("controls Visible", this->controls_visible);
    if (this->media_home != this->ia_config.value("media_home", QDir().absolutePath()).toString())
        this->ia_config.setValue("media_home", this->media_home);

    this->openauto_config->save();
}

Config *Config::get_instance()
{
    static Config config;
    return &config;
}
