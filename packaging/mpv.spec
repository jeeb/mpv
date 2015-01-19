Name:           mpv
Version:        0.7.999
Release:        0
Summary:        Video Player based on mplayer/mplayer2

Group:          Applications/Multimedia
License:        GPL-2.0+
URL:            https://%{name}.io
Source0:        %{name}-%{version}.tar.bz2
Source1001:     waf

BuildRequires: pkgconfig
BuildRequires: lua-devel >= 5.1.4
BuildRequires: ffmpeg-mpv-devel
BuildRequires: python
BuildRequires: libass-devel
BuildRequires: weston-ivi-shell
BuildRequires: weston-ivi-shell-devel
BuildRequires: pkgconfig(wayland-client) >= 1.6
BuildRequires: pkgconfig(wayland-cursor) >= 1.6
BuildRequires: pkgconfig(xkbcommon) >= 0.3
BuildRequires: pkgconfig(libpulse)
BuildRequires: pkgconfig(wayland-egl) >= 9.0.0
BuildRequires: pkgconfig(egl) >= 9.0.0

Requires: ffmpeg-mpv

%description
The %{name} video player, based upon mplayer/mplayer2.

%prep
%setup -q
cp %{SOURCE1001} .

%build
./waf configure \
    --prefix=%{_prefix} \
    --confdir=%{_sysconfdir}/mpv \
    --disable-manpage-build \
    --enable-gl-wayland
./waf build

%install
rm -rf %{buildroot}
ls -alh --color=auto
./waf install --destdir=$RPM_BUILD_ROOT

%files
%{_bindir}/mpv
%config %{_sysconfdir}/mpv/encoding-profiles.conf
%{_datadir}/applications/mpv.desktop
%config %{_datadir}/doc/mpv/*
%{_datadir}/icons/hicolor/*/apps/mpv.png
