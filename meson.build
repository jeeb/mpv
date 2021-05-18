project('mpv',
        'c',
        license: ['GPL2+', 'LGPL2.1+'],
        version: '0.33',
        meson_version: '>=0.59.0', #TODO: bump this to 0.60 when it releases
        default_options: [
            'buildtype=debugoptimized',
            'c_std=c11',
            'warning_level=1',
        ]
)

cc = meson.get_compiler('c')

# essential dependencies
ffmpeg = {
    'name': 'ffmpeg',
    'deps': [dependency('libavutil', version: '>= 56.12.100'),
             dependency('libavcodec', version: '>= 58.12.100'),
             dependency('libavformat', version: '>= 58.9.100'),
             dependency('libswscale', version: '>= 5.0.101'),
             dependency('libavfilter', version: '>= 7.14.100'),
             dependency('libswresample', version: '>= 3.0.100')],
}

libass = dependency('libass', version: '>= 0.12.2')
pthreads = dependency('threads')

dependencies = [ffmpeg['deps'],
                libass,
                pthreads]

features = [ffmpeg['name'], libass.name(), pthreads.name()]

# Builtin options we'd like to add to features.
features += get_option('default_library')
features += get_option('prefix')

if get_option('optimization') != '0'
    features += 'optimize'
endif

if get_option('debug')
    features += 'debug'
endif


# compiler/linker flags
flags = ['-D_ISOC99_SOURCE', '-D_GNU_SOURCE',
         '-D_FILE_OFFSET_BITS=64']
link_flags = []

test_flags = ['-Werror=implicit-function-declaration',
              '-Wno-error=deprecated-declarations',
              '-Wno-error=unused-function',
              '-Wempty-body',
              '-Wdisabled-optimization',
              '-Wstrict-prototypes',
              '-Wno-format-zero-length',
              '-Werror=format-security',
              '-Wno-redundant-decls',
              '-Wvla',
              '-Wno-format-truncation',
              '-Wimplicit-fallthrough',
              '-fno-math-errno']

flags += cc.get_supported_arguments(test_flags)

if cc.get_id() == 'gcc'
    gcc_flags = ['-Wundef', '-Wmissing-prototypes', '-Wshadow',
                 '-Wno-switch', '-Wparentheses', '-Wpointer-arith',
                 '-Wno-pointer-sign',
                 # GCC bug 66425
                 '-Wno-unused-result']
    flags += gcc_flags
endif

if cc.get_id() == 'clang'
    clang_flags = ['-Wno-logical-op-parentheses',
                   '-Wno-tautological-compare',
                   '-Wno-tautological-constant-out-of-range-compare']
    flags += clang_flags
endif

fragments = join_paths(meson.project_source_root(), 'waftools', 'fragments')
vector = cc.compiles(files(join_paths(fragments, 'vector.c')), name: 'vector check')
if vector
    features += 'vector'
elif get_option('vector').enabled()
    error('vector enabled but the compiler check failed!')
endif

win32 = host_machine.system() == 'cygwin' or host_machine.system() == 'windows'
posix = false
if not win32
    posix = true
    features += ['posix']
else
    features += 'win32'
endif

mswin_flags = ['-D_WIN32_WINNT=0x0602', '-DUNICODE', '-DCOBJMACROS',
               '-DINITGUID', '-U__STRICT_ANSI__']

mswin_linkflags = ['-Wl,--major-os-version=6,--minor-os-version=0',
                   '-Wl,--major-subsystem-version=6,--minor-subsystem-version=0']

if host_machine.system() == 'windows'
    flags += [mswin_flags, '-D__USE_MINGW_ANSI_STDIO=1']
    link_flags += [mswin_linkflags, '-mwindows']
endif

if host_machine.system() == 'cygwin'
    features += 'cygwin'
    flags += [mswin_flags, '-mwin32']
    link_flags += mswin_linkflags
endif

noexecstack = false
if cc.has_link_argument('-Wl,-z,noexecstack')
    link_flags += '-Wl,-z,noexecstack'
    noexecstack = true
endif

if cc.has_link_argument('-Wl,--nxcompat,--no-seh,--dynamicbase')
    link_flags += '-Wl,--nxcompat,--no-seh,--dynamicbase'
    noexecstack = true
endif

if noexecstack
    features += 'noexecstack'
endif

if not get_option('build-date')
    flags += '-DNO_BUILD_TIMESTAMPS'
else
    features += 'build-date'
endif

if get_option('ta-leak-report')
    features += 'ta-leak-report'
endif

if get_option('ffmpeg-strict-abi')
    features += 'ffmpeg-strict-abi'
endif

libdl = cc.find_library('libdl', required: false)
if libdl.found()
    dependencies += libdl
    features += 'libdl'
endif

cplugins = {
    'name': 'cplugins',
    'use': libdl.found() and not win32 and get_option('cplugins').allowed()
}
if cplugins['use']
    if cc.has_link_argument('-rdynamic')
        features += cplugins['name']
        link_flags += '-rdynamic'
    elif get_option('cplugins').enabled()
        error('cplugins enabled but not supported by the os/compiler!')
    endif
endif


# various file generations
tools_directory = join_paths(meson.project_source_root(), 'TOOLS')
file2string = find_program(join_paths(tools_directory, 'file2string.py'))
matroska = find_program(join_paths(tools_directory, 'matroska.py'))
version_py = find_program(join_paths(meson.project_source_root(), 'version.py'))


# generic sources
sources = files(
    ## Audio
    'audio/aframe.c',
    'audio/chmap.c',
    'audio/chmap_sel.c',
    'audio/decode/ad_lavc.c',
    'audio/decode/ad_spdif.c',
    'audio/filter/af_drop.c',
    'audio/filter/af_format.c',
    'audio/filter/af_lavcac3enc.c',
    'audio/filter/af_scaletempo.c',
    'audio/filter/af_scaletempo2.c',
    'audio/filter/af_scaletempo2_internals.c',
    'audio/fmt-conversion.c',
    'audio/format.c',
    'audio/out/ao.c',
    'audio/out/ao_lavc.c',
    'audio/out/ao_null.c',
    'audio/out/ao_pcm.c',
    'audio/out/buffer.c',

    ## Core
    'common/av_common.c',
    'common/av_log.c',
    'common/codecs.c',
    'common/common.c',
    'common/encode_lavc.c',
    'common/msg.c',
    'common/playlist.c',
    'common/recorder.c',
    'common/stats.c',
    'common/tags.c',
    'common/version.c',

    ## Demuxers
    'demux/codec_tags.c',
    'demux/cue.c',
    'demux/cache.c',
    'demux/demux.c',
    'demux/demux_cue.c',
    'demux/demux_disc.c',
    'demux/demux_edl.c',
    'demux/demux_lavf.c',
    'demux/demux_mf.c',
    'demux/demux_mkv.c',
    'demux/demux_mkv_timeline.c',
    'demux/demux_null.c',
    'demux/demux_playlist.c',
    'demux/demux_raw.c',
    'demux/demux_timeline.c',
    'demux/ebml.c',
    'demux/packet.c',
    'demux/timeline.c',

    'filters/f_async_queue.c',
    'filters/f_autoconvert.c',
    'filters/f_auto_filters.c',
    'filters/f_decoder_wrapper.c',
    'filters/f_demux_in.c',
    'filters/f_hwtransfer.c',
    'filters/f_lavfi.c',
    'filters/f_output_chain.c',
    'filters/f_swresample.c',
    'filters/f_swscale.c',
    'filters/f_utils.c',
    'filters/filter.c',
    'filters/frame.c',
    'filters/user_filters.c',

    ## Input
    'input/cmd.c',
    'input/event.c',
    'input/input.c',
    'input/ipc.c',
    'input/keycodes.c',

    ## Misc
    'misc/bstr.c',
    'misc/charset_conv.c',
    'misc/dispatch.c',
    'misc/json.c',
    'misc/natural_sort.c',
    'misc/node.c',
    'misc/rendezvous.c',
    'misc/thread_pool.c',
    'misc/thread_tools.c',

    ## Options
    'options/m_config_core.c',
    'options/m_config_frontend.c',
    'options/m_option.c',
    'options/m_property.c',
    'options/options.c',
    'options/parse_commandline.c',
    'options/parse_configfile.c',
    'options/path.c',

    ## Player
    'player/audio.c',
    'player/client.c',
    'player/command.c',
    'player/configfiles.c',
    'player/external_files.c',
    'player/loadfile.c',
    'player/main.c',
    'player/misc.c',
    'player/osd.c',
    'player/playloop.c',
    'player/screenshot.c',
    'player/scripting.c',
    'player/sub.c',
    'player/video.c',

    ## Streams
    'stream/cookies.c',
    'stream/stream.c',
    'stream/stream_avdevice.c',
    'stream/stream_cb.c',
    'stream/stream_concat.c',
    'stream/stream_edl.c',
    'stream/stream_file.c',
    'stream/stream_lavf.c',
    'stream/stream_memory.c',
    'stream/stream_mf.c',
    'stream/stream_null.c',
    'stream/stream_slice.c',

    ## Subtitles
    'sub/ass_mp.c',
    'sub/dec_sub.c',
    'sub/draw_bmp.c',
    'sub/filter_sdh.c',
    'sub/img_convert.c',
    'sub/lavc_conv.c',
    'sub/osd.c',
    'sub/osd_libass.c',
    'sub/sd_ass.c',
    'sub/sd_lavc.c',

    ## Video
    'video/csputils.c',
    'video/decode/vd_lavc.c',
    'video/filter/refqueue.c',
    'video/filter/vf_format.c',
    'video/filter/vf_sub.c',
    'video/fmt-conversion.c',
    'video/hwdec.c',
    'video/image_loader.c',
    'video/image_writer.c',
    'video/img_format.c',
    'video/mp_image.c',
    'video/mp_image_pool.c',
    'video/out/aspect.c',
    'video/out/bitmap_packer.c',
    'video/out/dither.c',
    'video/out/dr_helper.c',
    'video/out/filter_kernels.c',
    'video/out/gpu/context.c',
    'video/out/gpu/error_diffusion.c',
    'video/out/gpu/hwdec.c',
    'video/out/gpu/lcms.c',
    'video/out/gpu/libmpv_gpu.c',
    'video/out/gpu/osd.c',
    'video/out/gpu/ra.c',
    'video/out/gpu/shader_cache.c',
    'video/out/gpu/spirv.c',
    'video/out/gpu/user_shaders.c',
    'video/out/gpu/utils.c',
    'video/out/gpu/video.c',
    'video/out/gpu/video_shaders.c',
    'video/out/libmpv_sw.c',
    'video/out/vo.c',
    'video/out/vo_gpu.c',
    'video/out/vo_image.c',
    'video/out/vo_lavc.c',
    'video/out/vo_libmpv.c',
    'video/out/vo_null.c',
    'video/out/vo_tct.c',
    'video/out/win_state.c',
    'video/repack.c',
    'video/sws_utils.c',

    ## osdep
    'osdep/io.c',
    'osdep/threads.c',
    'osdep/timer.c',

    'osdep/semaphore_osx.c',
    'osdep/subprocess.c',

    ## tree_allocator
    'ta/ta.c',
    'ta/ta_talloc.c',
    'ta/ta_utils.c'
)

if get_option('tests')
    features += 'tests'
    sources += ['test/champ.c',
                'test/gl_video.c',
                'test/img_format.c',
                'test/json.c',
                'test/linked_list.c',
                'test/paths.c',
                'test/scale_sws.c',
                'test/scale_test.c',
                'test/tests.c']
endif

includedir = []
win32_pthreads = win32 and not posix and get_option('win32-internal-pthreads').allowed()
if win32_pthreads
    features += 'win32-internal-pthreads'
    flags += ['-isystem', '-I', '-DIN_WINPTHREAD']
    # Note: Adding this include causes POSIX_TIMERS to be defined for
    # unclear reasons (some confusion with <pthread.h> probably).
    # Hack around it by using HAVE_WIN32_INTERNAL_PTHREADS.
    includedir += include_directories('osdep/win32/include')
    sources += 'osdep/win32/pthread.c'
elif get_option('win32-internal-pthreads').enabled()
    error('win32-internal-pthreads enabled but the os is not win32!')
endif

if get_option('pthread-debug').allowed() and not win32_pthreads
    features += 'pthread-debug'
    flags += '-DMP_PTHREAD_DEBUG'
elif get_option('pthread-debug').enabled()
    error('pthread-debug enabled but win32-internal-threads was found!')
endif

add_project_arguments(flags, language: 'c')
add_project_link_arguments(link_flags, language: 'c')


# osdep
if posix
    sources += ['input/ipc-unix.c',
                'osdep/main-fn-unix.c',
                'osdep/path-unix.c',
                'osdep/polldev.c',
                'osdep/subprocess-posix.c',
                'osdep/terminal-unix.c',
                'sub/filter_regex.c']
endif

darwin = false
if host_machine.system() == 'darwin'
    darwin = true
    features += 'darwin'
    sources += 'osdep/timer-darwin.c'
endif

if posix and not darwin
    sources += 'osdep/timer-linux.c'
endif

cd_devices = {
    'windows': 'D:',
    'cygwin': 'D:',
    'darwin': '/dev/disk1',
    'freebsd': '/dev/cd0',
    'openbsd': '/dev/rcd0c',
    'linux': '/dev/sr0',
}
if host_machine.system() in cd_devices
    cd_device = cd_devices[host_machine.system()]
else
    cd_device = '/dev/cdrom'
endif

dvd_devices = {
    'windows': 'D:',
    'cygwin': 'D:',
    'darwin': '/dev/diskN',
    'freebsd': '/dev/cd0',
    'openbsd': '/dev/rcd0c',
    'linux': '/dev/sr0',
}
if host_machine.system() in cd_devices
    dvd_device = dvd_devices[host_machine.system()]
else
    dvd_device = '/dev/dvd'
endif

android = host_machine.system() == 'android'
if android
    features += 'android'
    sources += ['audio/out/ao_audiotrack.c',
                'video/out/android_common.c',
                'video/out/vo_mediacodec_embed.c']
endif

uwp = {
    'name': 'uwp',
    'deps': cc.find_library('windowsapp', required: get_option('uwp')),
    'use': false,
}
if uwp['deps'].found() and not get_option('cplayer')
    dependencies += uwp
    features += 'uwp'
    sources += 'osdep/path-uwp.c'
elif get_option('uwp').enabled()
    error('''uwp enabled but either windows app was not found or
          cplayer is not false!''')
endif

if win32
    sources += ['osdep/timer-win2.c',
                'osdep/w32_keyboard.c',
                'osdep/windows_utils.c']
endif

win32_desktop = win32 and not uwp['use']
if win32_desktop
    win32_desktop_libs = [cc.find_library('avrt'),
                          cc.find_library('dwmapi'),
                          cc.find_library('gdi32'),
                          cc.find_library('ole32'),
                          cc.find_library('uuid'),
                          cc.find_library('version'),
                          cc.find_library('winmm')]
    dependencies += win32_desktop_libs
    features += 'win32-desktop'
    sources += ['input/ipc-win.c',
                'osdep/main-fn-win.c',
                'osdep/path-win.c',
                'osdep/subprocess-win.c',
                'osdep/terminal-win.c',
                'video/out/w32_common.c',
                'video/out/win32/displayconfig.c',
                'video/out/win32/droptarget.c']
endif

if not posix and not win32_desktop
    sources += ['input/ipc-dummy.c',
                'osdep/subprocess-dummy.c',
                'osdep/terminal-dummy.c']
endif

glob_posix = cc.compiles(files(join_paths(fragments, 'glob_posix.c')), name: 'glob-posix check')
if glob_posix
    features += 'glob_posix'
endif

glob_win32 = win32 and not posix
if glob_win32
    features += 'glob_win32'
    sources += 'osdep/glob-win.c'
endif

glob = glob_posix or glob_win32
if glob
    features += 'glob'
endif

vt_h = cc.compiles(files(join_paths(fragments, 'vt_h.c')), name: 'vt.h check')
if vt_h
    features += 'vt.h'
endif

consio_h = cc.compiles(files(join_paths(fragments, 'consio_h.c')), name: 'consio.h check')
if consio_h
    features += 'consio.h'
endif

glibc_thread_name = cc.compiles(files(join_paths(fragments, 'glibc_thread_name.c')), name: 'glibc-thread-name check')
if glibc_thread_name and posix
    features += 'glibc-thread-name'
endif

osx_thread_name = cc.compiles(files(join_paths(fragments, 'osx_thread_name.c')), name: 'osx-thread-name check')
if osx_thread_name and darwin
    features += 'osx-thread-name'
endif

bsd_thread_name = cc.compiles(files(join_paths(fragments, 'bsd_thread_name.c')), name: 'bsd-thread-name check')
if bsd_thread_name and posix
    features += 'bsd-thread-name'
endif

bsd_fstatfs = cc.compiles(files(join_paths(fragments, 'bsd_fstatfs.c')), name: 'bsd-fstatfs check')
if bsd_fstatfs
    features += 'bsd-fstatfs'
endif

linux_fstatfs = cc.compiles(files(join_paths(fragments, 'linux_fstatfs.c')), name: 'linux-fstatfs check')
if linux_fstatfs
    features += 'linux-fstatfs'
endif


subdir('generated')
subdir(join_paths('generated', 'etc'))
subdir(join_paths('generated', 'sub'))

if host_machine.system() == 'darwin'
    subdir(join_paths('generated', 'TOOLS', 'osxbundle', 'mpv.app', 'Contents', 'Resources'))
endif


# misc dependencies
cdda = dependency('libcdio_paranoia', required: get_option('cdda'))
if cdda.found()
    dependencies += cdda
    features += cdda.name()
    sources += 'stream/stream_cdda.c'
endif

dvbin = get_option('gpl') and get_option('dvbin').allowed()
if dvbin
    features += 'dvbin'
    sources += ['stream/dvb_tune.c',
                'stream/stream_dvb.c']
elif get_option('dvbin').enabled()
    error('dvbin enabled but gpl is not true!')
endif

dvdnav = dependency('dvdnav', version: '>= 4.2.0', required: get_option('dvdnav'))
dvdread = dependency('dvdread', version: '>= 4.1.0', required: get_option('dvdnav'))
if dvdnav.found() and dvdread.found() and get_option('gpl')
    dependencies += [dvdnav, dvdread]
    features += dvdnav.name()
    sources += 'stream/stream_dvdnav.c'
endif

iconv = dependency('iconv', required: get_option('iconv'))
if iconv.found()
    dependencies += iconv
    features += iconv.name()
endif

javascript = dependency('mujs', version: '>= 1.0.0', required: get_option('javascript'))
if javascript.found()
    dependencies += javascript
    features += javascript.name()
    sources += ['player/javascript.c',
                'sub/filter_jsre.c']
    subdir(join_paths('generated', 'player', 'javascript'))
endif

lcms2 = dependency('lcms2', version: '>= 2.6', required: get_option('lcms2'))
if lcms2.found()
    dependencies += lcms2
    features += lcms2.name()
endif

libarchive = dependency('libarchive', version: '>= 3.4.0', required: get_option('libarchive'))
if libarchive.found()
    dependencies += libarchive
    features += libarchive.name()
    sources += ['demux/demux_libarchive.c',
                'stream/stream_libarchive.c']
endif

libavdevice = dependency('libavdevice', version: '>= 57.0.0', required: get_option('libavdevice'))
if libavdevice.found()
    dependencies += libavdevice
    features += libavdevice.name()
endif

libbluray = dependency('libbluray', version: '>= 0.3.0', required: get_option('libbluray'))
if libbluray.found()
    dependencies += libbluray
    features += libbluray.name()
    sources += 'stream/stream_bluray.c'
endif

libm = cc.find_library('m', required: false)
if libm.found()
    dependencies += libm
    features += 'libm'
endif

librt = cc.find_library('rt', required: false)
if librt.found()
    dependencies += librt
    features += 'librt'
endif

lua = {
    'name': 'lua',
    'use': false,
}
lua_opt = get_option('lua')
if lua_opt != 'disabled'
    lua_version = [['52', '>= 5.2.0, < 5.3.0'],
                   ['lua52', '>= 5.2.0'],
                   ['lua5.2', '>= 5.2.0'],
                   ['lua-5.2', '>= 5.2.0'],
                   ['luajit', '>= 2.0.0'],
                   ['luadef51', '>= 5.1.0, < 5.2.0'],
                   ['51', '>= 5.1.0, < 5.2.0'],
                   ['lua51', '>= 5.1.0'],
                   ['lua5.1', '>= 5.1.0'],
                   ['lua-5.1', '>= 5.1.0']]
    foreach version : lua_version
        if lua_opt == 'auto' or lua_opt == 'enabled'
            lua += {'deps': dependency(version[0], version: version[1], required: false)}
            if lua['deps'].found()
                lua += {'use': true}
                break
            endif
        elif lua_opt == version[0]
            lua += {'deps': dependency(version[0], version: version[1])}
            if lua['deps'].found()
                lua += {'use': true}
                break
            endif
        endif
    endforeach
endif
if lua['use']
    dependencies += lua['deps']
    features += lua['name']
    sources += 'player/lua.c'
    subdir(join_paths('generated', 'player', 'lua'))
endif
if not lua['use'] and lua_opt == 'enabled'
     error('lua enabled but no suitable lua version could be found!')
endif

rubberband = dependency('rubberband', version: '>= 1.8.0', required: get_option('rubberband'))
if rubberband.found()
    dependencies += rubberband
    features += rubberband.name()
    sources += 'audio/filter/af_rubberband.c'
endif

sdl2 = dependency('sdl2', required: get_option('sdl2'))
if sdl2.found()
    dependencies += sdl2
    features += sdl2.name()
endif

sdl2_gamepad = sdl2.found() and get_option('sdl2-gamepad').allowed()
if sdl2_gamepad
    features += 'sdl2-gamepad'
    sources += 'input/sdl_gamepad.c'
elif get_option('sdl2-gamepad').enabled()
    error('sdl2-gamepad enabled but sdl2 was not found!')
endif

stdatomic = cc.find_library('atomic', required: get_option('stdatomic'))
if stdatomic.found()
    dependencies += stdatomic
    features += 'stdatomic'
endif

uchardet = dependency('uchardet', required: get_option('uchardet'))
if iconv.found() and uchardet.found()
    dependencies += uchardet
    features += uchardet.name()
endif

vapoursynth = dependency('vapoursynth', version: '>= 24', required: get_option('vapoursynth'))
vapoursynth_script = dependency('vapoursynth-script', version: '>= 23', required: get_option('vapoursynth'))
if vapoursynth.found() and vapoursynth_script.found()
    dependencies += [vapoursynth, vapoursynth_script]
    features += vapoursynth.name()
    sources += 'video/filter/vf_vapoursynth.c'
endif

zimg = dependency('zimg', version: '>= 2.9', required: get_option('zimg'))
if zimg.found()
    dependencies += zimg
    features += zimg.name()
    sources += ['video/filter/vf_fingerprint.c',
                'video/zimg.c']
    if get_option('tests')
        sources += ['test/repack.c',
                    'test/scale_zimg.c']
    endif
endif

zlib = dependency('zlib', required: get_option('zlib'))
if zlib.found()
    dependencies += zlib
    features += zlib.name()
endif


# audio output dependencies
alsa = dependency('alsa', version: '>= 1.0.18', required: get_option('alsa'))
if alsa.found()
    dependencies += alsa
    features += alsa.name()
    sources += 'audio/out/ao_alsa.c'
endif

audiounit = {
    'name': 'audiounit',
    'fragment': files(join_paths(fragments, 'audiounit.c')),
    'use': false,
}
if get_option('audiounit').allowed()
    audiounit += {'use': cc.compiles(audiounit['fragment'], name: 'audiounit check')}
endif
if audiounit['use']
    features += audiounit['name']
    sources += 'audio/out/ao_audiounit.m'
elif get_option('audiounit').enabled()
    error('audiounit enabled but the compiler check failed!')
endif

coreaudio = {
    'name': 'coreaudio',
    'fragment': files(join_paths(fragments, 'coreaudio.c')),
    'use': false,
}
if get_option('coreaudio').allowed()
    coreaudio += {'use': cc.compiles(coreaudio['fragment'], name: 'coreaudio check')}
endif
if coreaudio['use']
    features += coreaudio['name']
    sources += ['audio/out/ao_coreaudio.c',
                'audio/out/ao_coreaudio_exclusive.c',
                'audio/out/ao_coreaudio_properties.c']
elif get_option('coreaudio').enabled()
    error('coreaudio enabled but the compiler check failed!')
endif

if coreaudio['use'] or audiounit['use']
    sources += ['audio/out/ao_coreaudio_chmap.c',
                'audio/out/ao_coreaudio_utils.c']
endif

jack = dependency('jack', required: get_option('jack'))
if jack.found() and get_option('gpl')
    dependencies += jack
    features += jack.name()
    sources += 'audio/out/ao_jack.c'
endif

openal = dependency('openal', version: '>= 1.13', required: get_option('openal'))
if openal.found()
    dependencies += openal
    features += openal.name()
    sources += 'audio/out/ao_openal.c'
endif

opensles = cc.find_library('OpenSLES', required: get_option('opensles'))
if opensles.found()
    dependencies += opensles
    features += 'opensles'
    sources += 'audio/out/ao_opensles.c'
endif

oss = {
    'name': 'oss',
    'fragment': files(join_paths(fragments, 'oss.c')),
    'use': false,
}
if get_option('oss-audio').allowed()
    oss += {'use': cc.compiles(oss['fragment'], name: 'oss check')}
endif
if oss['use']
    features += oss['name']
    sources += 'audio/out/ao_oss.c'
elif get_option('oss-audio').enabled()
    error('oss-audio enabled but the compiler check failed!')
endif

pulse = dependency('libpulse', version: '>= 1.0', required: get_option('pulse'))
if pulse.found()
    dependencies += pulse
    features += pulse.name()
    sources += 'audio/out/ao_pulse.c'
endif

sdl2_audio = sdl2.found() and get_option('sdl2-audio').allowed()
if sdl2_audio
    features += 'sdl2-audio'
    sources += 'audio/out/ao_sdl.c'
elif get_option('sdl2-audio').enabled()
    error('sdl2-audio enabled but sdl2 was not found!')
endif

wasapi = {
    'name': 'wasapi',
    'fragment': files(join_paths(fragments, 'wasapi.c')),
    'use': false,
}
if get_option('wasapi').allowed() and win32
    wasapi += {'use': cc.compiles(wasapi['fragment'], name: 'wasapi check')}
endif
if wasapi['use']
    features += wasapi['name']
    sources += ['audio/out/ao_wasapi.c',
                'audio/out/ao_wasapi_changenotify.c',
                'audio/out/ao_wasapi_utils.c']
elif get_option('wasapi').enabled()
    error('wasapi enabled but the compiler check failed or the os is not win32!')
endif


# video output dependencies
caca = dependency('caca', version: '>= 0.99.beta18', required: get_option('caca'))
if caca.found()
    dependencies += caca
    features += caca.name()
    sources += 'video/out/vo_caca.c'
endif

direct3d = {
    'name': 'direct3d',
    'header': cc.check_header('d3d9.h'),
}
direct3d += {'use': direct3d['header'] and win32_desktop and get_option('gpl') and get_option('direct3d').allowed()}
if direct3d['use']
    features += direct3d['name']
    sources += 'video/out/vo_direct3d.c'
elif get_option('direct3d').enabled()
    error('''direct3d enabled but either the header check failed, 
          this is not a win32 desktop or gpl is not true!''')
endif

drm = {
    'name': 'drm',
    'deps': dependency('libdrm', version: '>= 2.4.74', required: get_option('drm')),
    'header': vt_h or consio_h,
}
drm += {'use': drm['deps'].found() and drm['header']}
if drm['use']
    dependencies += drm['deps']
    features += drm['name']
    sources += ['video/out/drm_atomic.c',
                'video/out/drm_common.c',
                'video/out/drm_prime.c',
                'video/out/opengl/hwdec_drmprime_drm.c',
                'video/out/vo_drm.c']
endif

gbm = {
    'name': 'gbm',
    'deps': dependency('gbm', required: get_option('gbm')),
    'header': cc.check_header('stdio.h') and cc.check_header('gbm.h'),
}
gbm += {'use': gbm['deps'].found() and gbm['header']}
if gbm['use']
    dependencies += gbm['deps']
    features += gbm['name']
endif

jpeg = dependency('libjpeg', required: get_option('jpeg'))
if jpeg.found()
    dependencies += jpeg
    features += jpeg.name()
endif

libplacebo = dependency('libplacebo', version: '>=3.104.0', required: get_option('libplacebo'))
if libplacebo.found()
    dependencies += libplacebo
    features += libplacebo.name()
    sources += ['video/out/placebo/ra_pl.c',
                'video/out/placebo/utils.c']
    version = libplacebo.version()
    version_split = version.split('.')
    libplacebo_apiver = version_split[1]
else
    libplacebo_apiver = 0
endif

sdl2_video = sdl2.found() and get_option('sdl2-video').allowed()
if sdl2_video
    features += 'sdl2-video'
    sources += 'video/out/vo_sdl.c'
elif get_option('sdl2-video').enabled()
    error('sdl2-video enabled but sdl2 was not found!')
endif

shaderc = {
    'name': 'shaderc',
    'deps': dependency('shaderc', required: get_option('shaderc')),
    'header': cc.check_header('shaderc/shaderc.h'),
    'use': false,
}
shaderc += {'use': shaderc['deps'].found() and shaderc['header']}
if shaderc['use']
    dependencies += shaderc['deps']
    features += shaderc['name']
    sources += 'video/out/gpu/spirv_shaderc.c'
endif

sixel = dependency('libsixel', version: '>= 1.5', required: get_option('sixel'))
if sixel.found()
    dependencies += sixel
    features += sixel.name()
    sources += 'video/out/vo_sixel.c'
endif

spirv_cross = dependency('spirv-cross-c-shared', required: get_option('spirv-cross'))
if spirv_cross.found()
    features += spirv-cross.name()
endif

d3d11 = {
    'name': 'd3d11',
    'use': win32_desktop and shaderc['use'] and spirv_cross.found() and get_option('d3d11').allowed(),
}
if d3d11['use']
    features += d3d11['name']
    sources += ['video/out/d3d11/context.c',
                'video/out/d3d11/ra_d3d11.c']
elif get_option('d3d11').enabled()
    error('''d3d11 enabled but either this is not a win32 desktop,
          shaderc wasn't found or spirv-cross wasn't found!''')
endif

rpi = dependency('brcmegl', required: get_option('rpi'))

wayland = {
    'name': 'wayland',
    'deps': [dependency('wayland-client', version: '>= 1.15.0', required: get_option('wayland')),
             dependency('wayland-cursor', version: '>= 1.15.0', required: get_option('wayland')),
             dependency('wayland-protocols', required: get_option('wayland')),
             dependency('xkbcommon', version: '>= 0.3.0', required: get_option('wayland'))],
    'header': cc.check_header('linux/input-event-codes.h'),
    'scanner': find_program('wayland-scanner', required: get_option('wayland')),
    'use': false,
}
wayland_deps = true
foreach dep: wayland['deps']
    if not dep.found()
        wayland_deps = false
        break
    endif
endforeach

if wayland_deps and wayland['header'] and wayland['scanner'].found()
    wayland += {'use': true}
endif

if wayland['use']
    features += wayland['name']
    subdir(join_paths('generated', 'wayland'))
endif

memfd_create = {
    'name': 'memfd_create',
    'fragment': files(join_paths(fragments, 'memfd_create.c')),
    'use': false,
}
if wayland['use']
    memfd_create += {'use': cc.compiles(memfd_create['fragment'], name: 'memfd_create check')}
endif
if wayland['use'] and memfd_create['use']
    features += 'memfd_create'
    sources += 'video/out/vo_wlshm.c'
endif

x11 = {
    'name': 'x11',
    'deps': [dependency('x11', version: '>= 1.0.0', required: get_option('x11')),
             dependency('xscrnsaver', version: '>= 1.0.0', required: get_option('x11')),
             dependency('xext', version: '>= 1.0.0', required: get_option('x11')),
             dependency('xinerama', version: '>= 1.0.0', required: get_option('x11')),
             dependency('xrandr', version: '>= 1.2.0', required: get_option('x11'))],
}
foreach dep: x11['deps']
    if not dep.found()
        x11 += {'use': false}
        break
    endif
    if dep == x11['deps'][4]
       x11 += {'use': true}
    endif
endforeach
if x11['use']
    dependencies += x11['deps']
    features += x11['name']
    sources += ['video/out/vo_x11.c',
                'video/out/x11_common.c']
endif

xv = dependency('xv', required: get_option('xv'))
if xv.found()
    dependencies += xv
    features += xv.name()
    sources += 'video/out/vo_xv.c'
endif


# OpenGL junk
egl_helpers = {
    'name': 'egl-helpers',
    'use': false,
}

gl = {
    'name': 'gl',
    'use': false,
}

# For the various gl options, we need to error out
# if gl is disabled but one of these is enabled.
egl = {
    'name': 'egl',
    'deps': dependency('egl', version: '> 1.4.0', required: get_option('egl')),
    'use' : false,
}
if get_option('gl').disabled() and get_option('egl').enabled()
    error('egl is enabled but gl is disabled!')
endif

egl_android = {
    'name': 'egl-android',
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-android').enabled()
    error('egl-android is enabled but gl is disabled!')
endif

egl_angle = {
    'name': 'egl-angle',
    'fragment': files(join_paths(fragments, 'egl_angle.c')),
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-angle').enabled()
    error('egl-angle is enabled but gl is disabled!')
endif

egl_angle_lib = {
    'name': 'egl-angle-lib',
    'fragment': files(join_paths(fragments, 'egl_angle_lib.c')),
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-angle-lib').enabled()
    error('egl-angle-lib is enabled but gl is disabled!')
endif

egl_angle_win32 = {
    'name': 'egl-angle-win32',
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-angle-win32').enabled()
    error('egl-angle-win32 is enabled but gl is disabled!')
endif

egl_drm = {
    'name': 'egl-drm',
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-drm').enabled()
    error('egl-drm is enabled but gl is disabled!')
endif

egl_wayland = {
    'name': 'egl-wayland',
    'deps': dependency('wayland-egl', version: '>= 9.0.0', required: get_option('egl-wayland')),
    'use': false,
}
if get_option('gl').disabled() and get_option('egl-wayland').enabled()
    error('egl-wayland is enabled but gl is disabled!')
endif

egl_x11 = {
    'name': 'egl-x11',
    'use': x11['use'] and egl['use'] and get_option('egl-x11').allowed(),
}
if get_option('gl').disabled() and get_option('egl-x11').enabled()
    error('egl-x11 is enabled but gl is disabled!')
endif

gl_dxinterop = {
    'name': 'gl-dxinterop',
    'd3d_fragment': files(join_paths(fragments, 'gl_dxinterop_d3d.c')),
    'gl_fragment': files(join_paths(fragments, 'gl_dxinterop_gl.c')),
    'use': false,
}
if get_option('gl').disabled() and get_option('gl-dxinterop').enabled()
    error('gl-dxinterop is enabled but gl is disabled!')
endif

gl_win32 = {
    'name': 'gl-win32',
    'use': false,
}
if get_option('gl').disabled() and get_option('gl-win32').enabled()
    error('gl-win32 is enabled but gl is disabled!')
endif

gl_x11 = {
    'name': 'gl-x11',
    'use': false,
}
if get_option('gl').disabled() and get_option('gl-x11').enabled()
    error('gl-x11 is enabled but gl is disabled!')
endif

plain_gl = {
    'name': 'plain-gl',
    'use': get_option('plain-gl').allowed() and get_option('libmpv')
}
if get_option('gl').disabled() and get_option('plain-gl').enabled()
    error('plain-gl is enabled but gl is disabled!')
endif

# OpenGL dependency checking
if get_option('gl').allowed()
    sources += ['video/out/opengl/common.c',
                'video/out/opengl/context.c',
                'video/out/opengl/formats.c',
                'video/out/opengl/libmpv_gl.c',
                'video/out/opengl/ra_gl.c',
                'video/out/opengl/utils.c']

    GL = dependency('', required: false)
    if get_option('gl-x11').allowed()
        GL = dependency('GL', required: get_option('gl-x11'))
        dependencies += GL
    elif get_option('gl-win32').allowed()
        GL = dependency('GL', required: get_option('gl-win32'))
        dependencies += GL
    endif

    if GL.found() and win32_desktop
        dependencies += GL
        features += gl_win32['name']
        gl += {'use': true}
        gl_win32 += {'use': true}
        sources += 'video/out/opengl/context_win.c'
    endif

    if GL.found() and x11['use']
        dependencies += GL
        features += gl_x11['name']
        gl += {'use': true}
        gl_x11 += {'use': true}
        sources += 'video/out/opengl/context_glx.c'
    endif

    if gl_win32['use'] and get_option('gl-dxinterop').allowed()
        gl_dxinterop_d3d = cc.compiles(gl_dxinterop['d3d_fragment'], name: 'gl-dxinterop d3d check')
        gl_dxinterop_gl = cc.compiles(gl_dxinterop['gl_fragment'], name: 'gl-dxinterop gl check')
        gl_dxinterop += {'use': gl_dxinterop_d3d and gl_dxinterop_gl}
    endif
    if gl_dxinterop['use']
        features += gl_dxinterop['name']
        sources += 'video/out/opengl/context_dxinterop.c'
    elif get_option('gl-dxinterop').enabled()
        error('gl-dxinterop enabled but gl-win32 was not found or the compiler check failed!')
    endif

    if gl_win32['use'] and get_option('egl-angle').allowed()
        egl_angle += {'use': cc.compiles(egl_angle['fragment'], name: 'egl-angle check')}
    endif
    if egl_angle['use']
        features += egl_angle['name']
        sources += 'video/out/opengl/angle_dynamic.c'
    elif get_option('egl-angle').enabled()
        error('egl-angle enabled but gl-win32 was not found or the compiler check failed!')
    endif

    if egl_angle['use'] and get_option('egl-angle-lib').allowed()
        angle_flags = ['-DGL_APICALL=', '-DEGLAPI=',
                       '-DANGLE_NO_ALIASES', '-DANGLE_EXPORT=']
        angle_deps = [egl['deps'],
                      dependency('glesv2', required: get_option('egl-angle-lib')),
                      cc.find_library('dxguid', required: get_option('egl-angle-lib')),
                      cc.find_library('d3d9', required: get_option('egl-angle-lib')),
                      cc.find_library('gdi32', required: get_option('egl-angle-lib')),
                      cc.find_library('stdc++', required: get_option('egl-angle-lib'))]
        egl_angle_lib += {'use': cc.compiles(egl_angle_lib['fragment'], args: angle_flags,
                          dependencies: angle_deps), name: 'egl-angle-lib-check'}
        if egl_angle_lib['use']
            features += egl_angle_lib['name']
            dependencies += angle_deps
        elif get_option('egl-angle-lib').enabled()
            error('egl-angle-lib enabled but egl-angle was not found or the compiler check failed!')
        endif
    endif

    egl_angle_win32 += {'use': win32_desktop and egl_angle['use'] and
                        get_option('egl-angle-win32').allowed()}
    if egl_angle_win32['use']
        features += egl_angle_win32['name']
        sources += 'video/out/opengl/context_angle.c'
    elif get_option('egl-angle-win32').enabled()
        error('''egl-angle-win32 enabled but either this is not a win32 desktop
              or egl-angle was not found!''')
    endif

    if egl['deps'].found()
        dependencies += egl['deps']
        features += egl['name']
        egl += {'use': true}
    endif

    egl_android += {'use': android and egl['deps'].found() and get_option('egl-android').allowed()}
    if egl_android['use']
        features += egl_android['name']
        sources += 'video/out/opengl/context_android.c'
    elif get_option('egl-android').enabled()
        error('egl-android enabled but either this os is not android or egl was not found!')
    endif

    egl_drm += {'use': drm['use'] and gbm['use'] and egl['use'] and get_option('egl-drm').allowed()}
    if egl_drm['use']
        features += egl_drm['name']
        gl += {'use': true}
        sources += 'video/out/opengl/context_drm_egl.c'
    elif not egl_drm['use'] and get_option('egl-drm').enabled()
        error('egl-drm enabled but neither drm, gbm, nor egl could not be found!')
    endif

    egl_wayland += {'use': wayland['use'] and egl['use'] and egl_wayland['deps'].found()}
    if egl_wayland['use']
        dependencies += egl_wayland['deps']
        features += egl_wayland['name']
        gl += {'use': true}
        sources += 'video/out/opengl/context_wayland.c'
    endif

    if egl_x11['use']
        gl += {'use': true}
        features += egl_x11['name']
        sources += 'video/out/opengl/context_x11egl.c'
    elif get_option('egl-x11').enabled()
        error('egl-x11 enabled but neither x11 nor egl could not be found!')
    endif

    if egl_x11['use'] or gl_x11['use']
        sources += 'video/out/opengl/oml_sync.c'
    endif

    if plain_gl['use']
        features += plain_gl['name']
        gl += {'use': true}
    elif get_option('plain-gl').enabled()
        error('plain-gl enabled but libmpv is disabled!')
    endif

    if rpi.found()
        features += rpi.name()
        gl += {'use': true}
        sources += 'video/out/opengl/context_rpi.c'
    endif

    egl_helpers += {'use': egl['use'] or egl_android['use'] or egl_angle_win32['use'] or rpi.found()}
    if egl_helpers['use']
        features += egl_helpers['name']
        sources += 'video/out/opengl/egl_helpers.c'
    endif

    if egl['use'] and egl_helpers['use']
        sources += 'video/filter/vf_gpu.c'
    endif

    features += 'gl'
endif
if not gl['use'] and get_option('gl').allowed()
    error('gl enabled but no OpenGL video output could be found!')
endif

if d3d11['use'] or egl_angle_win32['use']
    sources += 'video/out/gpu/d3d11_helpers.c'
endif


# vulkan
vulkan = dependency('vulkan', required: get_option('vulkan'))
if vulkan.found() and libplacebo.found()
    dependencies += vulkan
    features += vulkan.name()
    sources += ['video/out/vulkan/context.c',
                'video/out/vulkan/context_display.c',
                'video/out/vulkan/utils.c']
endif

if vulkan.found() and libplacebo.found() and android
    sources += 'video/out/vulkan/context_android.c'
endif

if vulkan.found() and libplacebo.found() and wayland['use']
    sources += 'video/out/vulkan/context_wayland.c'
endif

if vulkan.found() and libplacebo.found() and win32_desktop
    sources += 'video/out/vulkan/context_win.c'
endif

if vulkan.found() and libplacebo.found() and x11['use']
     sources += 'video/out/vulkan/context_xlib.c'
endif


# hwaccel
ffnvcodec = dependency('ffnvcodec', version: '>= 8.2.15.7', required: false)
if ffnvcodec.found()
    dependencies += ffnvcodec
    features += ffnvcodec.name()
    sources += 'video/cuda.c'
endif

if ffnvcodec.found() and get_option('cuda-hwaccel').allowed()
    features += 'cuda-hwaccel'
    sources += 'video/out/hwdec/hwdec_cuda.c'
elif get_option('cuda-hwaccel').enabled()
    error('cuda-hwaccel enabled but ffnvcodec was not found!')
endif

if ffnvcodec.found() and get_option('cuda-interop').allowed() and gl['use']
    features += 'cuda-interop'
    sources += 'video/out/hwdec/hwdec_cuda_gl.c'
elif get_option('cuda-interop').enabled()
    error('cuda-interop enabled but ffnvcodec was not found!')
endif

if ffnvcodec.found() and vulkan.found() and libplacebo.found() and get_option('cuda-interop').allowed()
    sources += 'video/out/hwdec/hwdec_cuda_vk.c'
endif

d3d_hwaccel = get_option('d3d-hwaccel').allowed() and win32
if d3d_hwaccel
    features += 'd3d_hwaccel'
    sources += ['video/d3d.c',
                'video/filter/vf_d3d11vpp.c']
elif get_option('d3d-hwaccel').enabled()
    error('d3d-hwaccel enabled but the os is not win32!')
endif
if d3d_hwaccel and egl_angle['use']
    sources += 'video/out/opengl/hwdec_d3d11egl.c'
endif
if d3d_hwaccel and d3d11['use']
    sources += 'video/out/d3d11/hwdec_d3d11va.c'
endif

d3d9_hwaccel = get_option('d3d9-hwaccel').allowed() and d3d_hwaccel
if d3d9_hwaccel and egl_angle['use']
    features += 'd3d9_hwaccel'
    sources += 'video/out/opengl/hwdec_dxva2egl.c'
elif get_option('d3d9-hwaccel').enabled()
    error('d3d9-hwaccel enabled but d3d_hwaccel was not found!')
endif
if d3d9_hwaccel and d3d11['use']
    sources += 'video/out/d3d11/hwdec_dxva2dxgi.c'
endif

gl_dxinterop_d3d9 = get_option('gl-dxinterop-d3d9').allowed() and gl_dxinterop['use']
if gl_dxinterop_d3d9
    features += 'gl-dxinterop-d3d9'
    sources += 'video/out/opengl/hwdec_dxva2gldx.c'
elif get_option('gl-dxinterop-d3d9').enabled()
    error('gl-dxinterop-d3d9 enabled but gl-dxinterop was not found!')
endif

rpi_mmal = dependency('mmal', required: get_option('rpi-mmal'))
if rpi.found() and rpi_mmal.found()
    features += 'rpi-mmal'
    sources += ['video/out/opengl/hwdec_rpi.c',
                'video/out/vo_rpi.c']
endif

vaapi = {
    'name': 'vaapi',
    'deps': dependency('libva', version: '>= 1.1.0', required: get_option('vaapi')),
}
vaapi += {'use': vaapi['deps'].found() and libdl.found() and (x11['use'] or wayland['use'] or egl_drm['use'])}
if vaapi['use']
    dependencies += vaapi['deps']
    features += vaapi['name']
    sources += ['video/filter/vf_vavpp.c',
                'video/vaapi.c']
endif

vaapi_drm = {
    'name': 'vaapi-drm',
    'deps': dependency('libva-drm', version: '>= 1.1.0', required: get_option('vaapi-drm')),
}
vaapi_drm += {'use': vaapi['use'] and egl_drm['use'] and vaapi_drm['deps'].found()}
if vaapi_drm['use']
    features += vaapi_drm['name']
endif

vaapi_wayland = {
    'name': 'vaapi-wayland',
    'deps': dependency('libva-wayland', version: '>= 1.1.0', required: get_option('vaapi-wayland')),
}
vaapi_wayland += {'use': vaapi['use'] and egl_wayland['use'] and vaapi_wayland['deps'].found()}
if vaapi_wayland['use']
    features += vaapi_wayland['name']
endif

vaapi_x11 = {
    'name': 'vaapi-x11',
    'deps': dependency('libva-x11', version: '>= 1.1.0', required: get_option('vaapi-x11')),
}
vaapi_x11 += {'use': vaapi['use'] and x11['use'] and vaapi_x11['deps'].found()}
if vaapi_x11['use'] and get_option('gpl')
    dependencies += vaapi_x11['deps']
    features += vaapi_x11['name']
    sources += 'video/out/vo_vaapi.c'
endif

vaapi_x_egl = {
    'name': 'vaapi-x-egl',
    'use': vaapi_x11['use'] and egl_x11['use'],
}
if vaapi_x_egl['use']
    features += vaapi_x_egl['name']
endif

vaapi_egl = {
    'name': 'vaapi-egl',
    'use': vaapi_x_egl['use'] or vaapi_wayland['use'] or vaapi_drm['use'],
}
if vaapi_egl['use']
    dependencies += [vaapi_wayland['deps'], vaapi_drm['deps']]
    features += vaapi_egl['name']
    sources += 'video/out/hwdec/hwdec_vaapi_gl.c'
endif

vaapi_vulkan = {
    'name': 'vaapi-vulkan',
    'use': vaapi['use'] and libplacebo.found() and vulkan.found(),
}
if vaapi_vulkan['use']
    features += vaapi_vulkan['name']
    sources += 'video/out/hwdec/hwdec_vaapi_vk.c'
endif

if vaapi_egl['use'] or vaapi_vulkan['use']
    sources += 'video/out/hwdec/hwdec_vaapi.c'
endif

vdpau = dependency('vdpau', version: '>= 0.2', required: get_option('vdpau'))
if vdpau.found() and x11['use']
    dependencies += vdpau
    features += vdpau.name()
    sources += ['video/filter/vf_vdpaupp.c',
                'video/out/vo_vdpau.c',
                'video/vdpau.c',
                'video/vdpau_mixer.c']
endif
if vdpau.found() and gl_x11['use']
    features += 'vdpau-gl-x11'
    sources += 'video/out/opengl/hwdec_vdpau.c'
endif


# manpages
manpage = 'DOCS/man/mpv.rst'
rst2man = find_program('rst2man', required: get_option('manpage-build'))
if rst2man.found()
    features += 'manpage-build'
    mandir = get_option('mandir')
    custom_target('rst2man',
        input: manpage,
        output: 'mpv.1',
        command: [rst2man, '--strip-elements-with-class=contents', '@INPUT@', '@OUTPUT@'],
        install: true,
        install_dir: join_paths(mandir, 'man1')
    )
endif

rst2html = find_program('rst2html', required: get_option('html-build'))
if rst2html.found()
    datadir = get_option('datadir')
    features += 'html-build'
    custom_target('rst2html',
        input: manpage,
        output: 'mpv.html',
        command: [rst2html, manpage, '@INPUT@', '@OUTPUT@'],
        install: true,
        install_dir: join_paths(datadir, 'doc', 'mpv')
    )
endif

rst2pdf = find_program('rst2pdf', required: get_option('pdf-build'))
if rst2pdf.found()
    datadir = get_option('datadir')
    features += 'pdf-build'
    custom_target('rst2pdf',
        input: manpage,
        output: 'mpv.pdf',
        command: [rst2pdf, '-c', '-b', '1', '--repeat-table-rows', manpage, '-o', '@INPUT@', '@OUTPUT@'],
        install: true,
        install_dir: join_paths(datadir, 'doc', 'mpv')
    )
endif

if get_option('cplayer')
    features += 'cplayer'
    datadir = get_option('datadir')
    confdir = get_option('sysconfdir')

    conf_files = ['etc/mpv.conf', 'etc/input.conf',
                  'etc/mplayer-input.conf', 'etc/restore-old-bindings.conf']
    install_data(conf_files, install_dir: join_paths(datadir, 'doc', 'mpv'))

    bash_install_dir = join_paths(datadir, 'bash-completion', 'completions')
    install_data('etc/mpv.bash-completion', install_dir: bash_install_dir, rename: 'mpv')

    zsh_install_dir = join_paths(datadir, 'zsh', 'site-functions')
    install_data('etc/_mpv.zsh', install_dir: zsh_install_dir, rename: '_mpv')

    install_data('etc/mpv.desktop', install_dir: join_paths(datadir, 'applications'))
    install_data('etc/encoding-profiles.conf', install_dir: join_paths(confdir, 'mpv'))

    foreach size: ['16x16', '32x32', '64x64', '128x128']
        icon_dir = join_paths('datadir', 'icons', 'hicolor', size, 'apps')
        install_data('etc/mpv-icon-8bit-' + size + '.png', install_dir: icon_dir, rename: 'mpv.png')
    endforeach

    hicolor_dir = join_paths(datadir, 'icons', 'hicolor')
    install_data('etc/mpv-gradient.svg', install_dir: join_paths(hicolor_dir, 'scalable', 'apps'), rename: 'mpv.svg')
    install_data('etc/mpv-symbolic.svg', install_dir: join_paths(hicolor_dir, 'symbolc', 'apps'))
endif

if get_option('libmpv')
    features += 'libmpv'
    client_h_path = join_paths(meson.project_source_root(), 'libmpv', 'client.h')
    client_api_version_py = find_program(join_paths(meson.project_source_root(), 'TOOLS', 'client-api-version.py'))
    client_api_version = run_command(client_api_version_py).stdout()

    libmpv = library('mpv', sources, dependencies: dependencies, version: client_api_version, 
                     include_directories: includedir, install: true)
    pkg = import('pkgconfig')
    pkg.generate(libmpv)

    headers = ['libmpv/client.h', 'libmpv/opengl_cb.h', 'libmpv/render.h',
               'libmpv/render_gl.h', 'libmpv/stream_cb.h']
    install_headers(headers, subdir: 'mpv')
endif


if win32
    windows = import('windows')
    res_flags = ['--codepage=65001']
    res_includes = [meson.project_source_root(), meson.project_build_root()]
    resources = ['etc/mpv-icon-8bit-16x16.png',
                 'etc/mpv-icon-8bit-32x32.png',
                 'etc/mpv-icon-8bit-64x64.png',
                 'etc/mpv-icon-8bit-128x128.png',
                 'etc/mpv-icon.ico',
                 'osdep/mpv.exe.manifest']
    sources += windows.compile_resources('osdep/mpv.rc', args: res_flags, depend_files: resources,
                                         depends: version_h, include_directories: res_includes)
endif

# Get the string of all enabled features
feature_sort = find_program(join_paths(tools_directory, 'feature-sort.py'))
feature_str = run_command(feature_sort, features).stdout()

# Compatibility with the waf build
# Much of this could be removed in the future
conf_data = configuration_data()
conf_data.set_quoted('CONFIGURATION', 'meson')
conf_data.set_quoted('DEFAULT_DVD_DEVICE', dvd_device)
conf_data.set_quoted('DEFAULT_CDROM_DEVICE', cd_device)
conf_data.set_quoted('FULLCONFIG', feature_str)
conf_data.set10('HAVE_ALSA', alsa.found())
conf_data.set10('HAVE_ANDROID', android)
conf_data.set10('HAVE_AUDIOUNIT', audiounit['use'])
conf_data.set10('HAVE_BSD_FSTATFS', bsd_fstatfs)
conf_data.set10('HAVE_BSD_THREAD_NAME', bsd_thread_name and posix)
conf_data.set10('HAVE_CACA', caca.found())
conf_data.set10('HAVE_CDDA', cdda.found())
conf_data.set10('HAVE_COCOA', 0)
conf_data.set10('HAVE_CONSIO_H', consio_h)
conf_data.set10('HAVE_COREAUDIO', coreaudio['use'])
conf_data.set10('HAVE_CPLUGINS', cplugins['use'])
conf_data.set10('HAVE_CUDA_HWACCEL', ffnvcodec.found() and get_option('cuda-interop').allowed())
conf_data.set10('HAVE_CUDA_INTEROP', ffnvcodec.found() and gl['use'] and get_option('cuda-interop').allowed())
conf_data.set10('HAVE_D3D_HWACCEL', d3d_hwaccel)
conf_data.set10('HAVE_D3D9_HWACCEL', d3d9_hwaccel)
conf_data.set10('HAVE_D3D11', d3d11['use'])
conf_data.set10('HAVE_DIRECT3D', direct3d['use'])
conf_data.set10('HAVE_DOS_PATHS', win32)
conf_data.set10('HAVE_DRM', drm['use'])
conf_data.set10('HAVE_DVBIN', dvbin)
conf_data.set10('HAVE_DVDNAV', dvdnav.found() and dvdread.found() and get_option('gpl'))
conf_data.set10('HAVE_EGL', egl['use'])
conf_data.set10('HAVE_EGL_ANDROID', egl_android['use'])
conf_data.set10('HAVE_EGL_ANGLE', egl_angle['use'])
conf_data.set10('HAVE_EGL_ANGLE_LIB', egl_angle_lib['use'])
conf_data.set10('HAVE_EGL_ANGLE_WIN32', egl_angle_win32['use'])
conf_data.set10('HAVE_EGL_DRM', egl_drm['use'])
conf_data.set10('HAVE_EGL_HELPERS', egl_helpers['use'])
conf_data.set10('HAVE_EGL_X11', egl_x11['use'])
conf_data.set10('HAVE_FFMPEG_STRICT_ABI', get_option('ffmpeg-strict-abi'))
conf_data.set10('HAVE_GLIBC_THREAD_NAME', glibc_thread_name and posix)
conf_data.set10('HAVE_GL', gl['use'])
conf_data.set10('HAVE_GL_COCOA', 0)
conf_data.set10('HAVE_GL_DXINTEROP', gl_dxinterop['use'])
conf_data.set10('HAVE_GL_DXINTEROP_D3D9', gl_dxinterop_d3d9)
conf_data.set10('HAVE_GL_WAYLAND', egl_wayland['use'])
conf_data.set10('HAVE_GL_WIN32', gl_win32['use'])
conf_data.set10('HAVE_GL_X11', gl_x11['use'])
conf_data.set10('HAVE_GLOB', glob)
conf_data.set10('HAVE_GLOB_POSIX', glob_posix)
conf_data.set10('HAVE_GPL', get_option('gpl'))
conf_data.set10('HAVE_ICONV', iconv.found())
conf_data.set10('HAVE_IOS_GL', 0)
conf_data.set10('HAVE_JACK', jack.found())
conf_data.set10('HAVE_JAVASCRIPT', javascript.found())
conf_data.set10('HAVE_JPEG', jpeg.found())
conf_data.set10('HAVE_LCMS2', lcms2.found())
conf_data.set10('HAVE_LIBARCHIVE', libarchive.found())
conf_data.set10('HAVE_LIBAVDEVICE', libavdevice.found())
conf_data.set10('HAVE_LIBDL', libdl.found())
conf_data.set10('HAVE_LIBBLURAY', libbluray.found())
conf_data.set10('HAVE_LINUX_FSTATFS', linux_fstatfs)
conf_data.set10('HAVE_LUA', lua['use'])
conf_data.set10('HAVE_MACOS_10_11_FEATURES', 0)
conf_data.set10('HAVE_MACOS_10_14_FEATURES', 0)
conf_data.set10('HAVE_MACOS_COCOA_CB', 0)
conf_data.set10('HAVE_MACOS_MEDIA_PLAYER', 0)
conf_data.set10('HAVE_MACOS_TOUCHBAR', 0)
conf_data.set10('HAVE_MEMFD_CREATE', memfd_create['use'])
conf_data.set10('HAVE_OPENAL', openal.found())
conf_data.set10('HAVE_OPENSLES', opensles.found())
conf_data.set10('HAVE_OSS_AUDIO', oss['use'])
conf_data.set10('HAVE_OSX_THREAD_NAME', osx_thread_name and darwin)
conf_data.set10('HAVE_POSIX', posix)
conf_data.set10('HAVE_PULSE', pulse.found())
conf_data.set10('HAVE_RPI', rpi.found())
conf_data.set10('HAVE_RPI_MMAL', rpi.found() and rpi_mmal.found())
conf_data.set10('HAVE_RUBBERBAND', rubberband.found())
conf_data.set10('HAVE_SDL2', sdl2.found())
conf_data.set10('HAVE_SDL2_AUDIO', sdl2_audio)
conf_data.set10('HAVE_SDL2_GAMEPAD', sdl2_gamepad)
conf_data.set10('HAVE_SDL2_VIDEO', sdl2_video)
conf_data.set10('HAVE_SHADERC', shaderc['use'])
conf_data.set10('HAVE_SIXEL', sixel.found())
conf_data.set10('HAVE_STDATOMIC', stdatomic.found())
conf_data.set10('HAVE_TA_LEAK_REPORT', get_option('ta-leak-report'))
conf_data.set10('HAVE_TESTS', get_option('tests'))
conf_data.set10('HAVE_UCHARDET', iconv.found() and uchardet.found())
conf_data.set10('HAVE_UWP', uwp['use'])
conf_data.set10('HAVE_VAAPI', vaapi['use'])
conf_data.set10('HAVE_VAAPI_DRM', vaapi_drm['use'])
conf_data.set10('HAVE_VAAPI_EGL', vaapi_egl['use'])
conf_data.set10('HAVE_VAAPI_VULKAN', vaapi_vulkan['use'])
conf_data.set10('HAVE_VAAPI_WAYLAND', vaapi_wayland['use'])
conf_data.set10('HAVE_VAAPI_X11', vaapi_x11['use'])
conf_data.set10('HAVE_VAPOURSYNTH', vapoursynth.found() and vapoursynth_script.found())
conf_data.set10('HAVE_VECTOR', vector)
conf_data.set10('HAVE_VDPAU', vdpau.found() and x11['use'])
conf_data.set10('HAVE_VDPAU_GL_X11', vdpau.found() and gl_x11['use'])
conf_data.set10('HAVE_VIDEOTOOLBOX_GL', 0)
conf_data.set10('HAVE_VULKAN', libplacebo.found() and vulkan.found())
conf_data.set10('HAVE_WASAPI', wasapi['use'])
conf_data.set10('HAVE_WAYLAND', wayland['use'])
conf_data.set10('HAVE_WIN32_DESKTOP', win32_desktop)
conf_data.set10('HAVE_WIN32_INTERNAL_PTHREADS', win32_pthreads)
conf_data.set10('HAVE_X11', x11['use'])
conf_data.set10('HAVE_XV', xv.found())
conf_data.set10('HAVE_ZIMG', zimg.found())
conf_data.set10('HAVE_ZLIB', zlib.found())
conf_data.set_quoted('MPV_CONFDIR', join_paths(get_option('sysconfdir'), 'mpv'))
conf_data.set('PL_API_VER', libplacebo_apiver)
configure_file(output : 'config.h', configuration : conf_data)

# TODO: macOS stuff

#'osdep/macos/log_helper.swift'
#'osdep/macos/libmpv_helper.swift'
#'osdep/main-fn-cocoa.c'
#'osdep/macos/mpv_helper.swift'
#'osdep/macos/swift_extensions.swift'
#'osdep/macos/swift_compat.swift'
#'osdep/macos/remote_command_center.swift' 'macos-media-player'
#'video/out/mac/common.swift'
#'video/out/mac/view.swift'
#'video/out/mac/window.swift'
#'video/out/mac/title_bar.swift'
#'video/out/cocoa_cb_common.swift', 'macos-cocoa-cb'
#'video/out/mac/gl_layer.swift', 'macos-cocoa-cb'

#'video/out/cocoa/events_view.m',       'cocoa',
#'video/out/cocoa/video_view.m',        'cocoa',
#'video/out/cocoa/window.m',            'cocoa',
#'video/out/cocoa_common.m',            'cocoa',
#'video/out/opengl/context_cocoa.c',    'gl-cocoa',
#'video/out/opengl/hwdec_ios.m',        'ios-gl',
#'video/out/opengl/hwdec_osx.c',        'videotoolbox-gl',

#'osdep/macosx_application.m',          'cocoa',
#'osdep/macosx_events.m',               'cocoa',
#'osdep/macosx_menubar.m',              'cocoa',
#'osdep/macosx_touchbar.m',             'macos-touchbar',
#'osdep/path-macosx.m',                 'cocoa',

if get_option('cplayer')
    executable('mpv', sources, dependencies: dependencies,
               include_directories: includedir, install: true)
endif
