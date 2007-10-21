/*
 * Copyright (C) 2007 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <iostream>
#include <fstream>
#include <tnt/tntnet.h>
#include <tnt/worker.h>
#include <cxxtools/loginit.h>
#include <cxxtools/arg.h>
#include <cxxtools/inifile.h>

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    const char* HOME = getenv("HOME");
    std::string settingsfile;
    if (HOME)
    {
      settingsfile = HOME;
      settingsfile += '/';
    }
    settingsfile += ".TntReader";

    cxxtools::IniFile settings;
    
    try
    {
      settings = cxxtools::IniFile(settingsfile.c_str());
    }
    catch (const std::exception& e)
    {
      std::ofstream out(settingsfile.c_str());
      out << "[TntReader]\n"
             "port=8080\n"
             "localonly=1\n"
             "directory=.\n";
      out.close();
      settings = cxxtools::IniFile(settingsfile.c_str());
    }

    std::string listenIp = settings.getValue("TntReader", "listen", "");

    if (listenIp.empty())
    {
      std::string localonly = settings.getValue("TntReader", "localonly", "0");
      listenIp = localonly == "1" ? "127.0.0.1" : "0.0.0.0";
    }

    unsigned short port = settings.getValueT<unsigned short>("TntReader", "port", 8080);

    std::string directory = settings.getValue("TntReader", "directory", ".");

    tnt::Tntnet app;
    tnt::Worker::setEnableCompression(false);

    std::cout << "IP " << listenIp << " port " << port << std::endl;
    app.listen(listenIp, port);

    app.mapUrl("^/$",                          "redirect");
    app.mapUrl("tntnet.png",                   "tntnet_png");
    app.mapUrl("^/-/([^.]+)\\.(.*)$",          "$1_$2");
    app.mapUrl("^/~/([^.]+)$",                 "$1");
    app.mapUrl("^/~/([^.]+)\\.([^.]*)$",       "$1_$2");

    app.mapUrl("^/(.)/(.+)$", "zenocomp")
       .setPathInfo("$2")
       .pushArg("wikipedia.zeno")
       .pushArg("$1");

    app.mapUrl("^/(.+)/(.)/(.+)$", "zenocomp")
       .setPathInfo("$3")
       .pushArg("$1.zeno")
       .pushArg("$2");

    app.mapUrl("^/!/([0-9]+)$",          "$1", "number");

    tnt::Tntconfig config;
    config.setConfigValue("ZenoPath", directory);
    tnt::Comploader::configure(config);

    std::cout << "Wikipedia ist jetzt unter http://localhost:" << port << "/ verfügbar\n"
                 "Die Einstellunge können unter $HOME/.TntReader geändert werden" << std::endl;
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

