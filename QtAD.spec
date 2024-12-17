Name: QtAD
Version: 2.0.0	
Release: 1%{?dist}
Summary: areaDetector Viewer in Qt/OpenGL

Group: Application/Engineering
License: BSD
URL: https://github.com/paulscherrerinstitute/QtAD
Source0: https://github.com/paulscherrerinstitute/QtAD/archive/%{version}/%{name}-%{version}.tar.gz


BuildRequires: qt5-devel
Requires: qt5-qtbase-gui

%description
areaDetector Viewer in Qt/OpenGL


%prep
%setup -q %{name}-%{version}.tar.gz


%build
qmake-qt5 PREFIX=%{buildroot}/%{_bindir}
make %{?_smp_mflags}


%install
make install

%files
%{_bindir}/QtAD

%doc


%changelog

