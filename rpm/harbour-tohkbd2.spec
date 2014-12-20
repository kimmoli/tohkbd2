#
# tohkbd2-daemon spec
# (C) kimmoli 2014
#

Name:       harbour-tohkbd2

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    The OtherHalf Keyboard version 2 daemon
Version:    0.0.devel
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        https://github.com/kimmoli/tohkbd2-daemon
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  qt5-qttools-linguist

Requires:   harbour-tohkbd2-user
Requires:   harbour-tohkbd2-settingsui

%description
Daemon for The OtherHalf Keyboard (TOHKBD) version 2

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/%{name}
%{_datadir}/%{name}/
%{_datadir}/%{name}/i18n/
%{_sysconfdir}/systemd/system/
%{_sysconfdir}/udev/rules.d/
%{_sysconfdir}/dbus-1/system.d/
%{_datadir}/maliit/plugins/com/jolla/layouts/

%post
#reload udev rules
udevadm control --reload
# if tohkbd2 is connected, start daemon now
# vendor id 6537 = 0x1989 = dirkvl
# product id 3 = tohkbd2
if [ -e /sys/devices/platform/toh-core.0/vendor ]; then
 if grep -q 6537 /sys/devices/platform/toh-core.0/vendor ; then
  if grep -q 3 /sys/devices/platform/toh-core.0/product ; then
   systemctl start %{name}.service
  fi
 fi
fi

%pre
# In case of update, stop and disable first
if [ "$1" = "2" ]; then
  systemctl stop %{name}.service
  systemctl disable %{name}.service
  udevadm control --reload
fi

%preun
# in case of complete removal, stop and disable
if [ "$1" = "0" ]; then
  systemctl stop %{name}.service
  systemctl disable %{name}.service
  udevadm control --reload
fi
