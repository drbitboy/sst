
    % uname -a
    Linux ubuntu 5.10.104-tegra #1 SMP PREEMPT Wed Aug 10 20:17:07 PDT 2022 aarch64 aarch64 aarch64 GNU/Linux


    % cat /etc/os-release
    NAME="Ubuntu"
    VERSION="20.04.4 LTS (Focal Fossa)"
    ID=ubuntu
    ID_LIKE=debian
    PRETTY_NAME="Ubuntu 20.04.4 LTS"
    VERSION_ID="20.04"
    HOME_URL="https://www.ubuntu.com/"
    SUPPORT_URL="https://help.ubuntu.com/"
    BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
    PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
    VERSION_CODENAME=focal
    UBUNTU_CODENAME=focal


    % stty -a -F /dev/ttyTHS0
    speed 115200 baud; rows 0; columns 0; line = 0;
    intr = <undef>; quit = <undef>; erase = <undef>; kill = <undef>; eof = <undef>; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = <undef>; stop = <undef>;
    susp = <undef>; rprnt = <undef>; werase = <undef>; lnext = <undef>; discard = <undef>; min = 1; time = 0;
    parenb -parodd -cmspar cs8 -hupcl cstopb cread clocal crtscts
    -ignbrk -brkint -ignpar -parmrk inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -iutf8
    -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
    -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke -flusho -extproc


    % cat /proc/cpuinfo
    processor       : 0
    model name      : ARMv8 Processor rev 0 (v8l)
    BogoMIPS        : 62.50
    Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm dcpop
    CPU implementer : 0x4e
    CPU architecture: 8
    CPU variant     : 0x0
    CPU part        : 0x004
    CPU revision    : 0
    MTS version     : 55637613

    processor       : 1
    model name      : ARMv8 Processor rev 0 (v8l)
    BogoMIPS        : 62.50
    Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm dcpop
    CPU implementer : 0x4e
    CPU architecture: 8
    CPU variant     : 0x0
    CPU part        : 0x004
    CPU revision    : 0
    MTS version     : 55637613

    processor       : 2
    model name      : ARMv8 Processor rev 0 (v8l)
    BogoMIPS        : 62.50
    Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm dcpop
    CPU implementer : 0x4e
    CPU architecture: 8
    CPU variant     : 0x0
    CPU part        : 0x004
    CPU revision    : 0
    MTS version     : 55637613

    processor       : 3
    model name      : ARMv8 Processor rev 0 (v8l)
    BogoMIPS        : 62.50
    Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm dcpop
    CPU implementer : 0x4e
    CPU architecture: 8
    CPU variant     : 0x0
    CPU part        : 0x004
    CPU revision    : 0
    MTS version     : 55637613
