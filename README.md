# fty-common-rest
Library providing :
* Common methods and functions for REST API, or which require use of tntnet

## How to Build
```bash
./autogen.sh [clean]
./configure
make
make check # to run self-test
```

## List of Available Headers
* fty\_common\_rest\_audit\_log.h
* fty\_common\_rest\_helpers.h
* fty\_common\_rest\_sasl.h
* fty\_common\_rest\_utils\_web.h
* fty\_common\_rest.h
* fty\_common\_rest\_library.h
* fty\_common\_rest\_tokens.h

## How to compile using fty-common-rest

### project.xml
Add this block in the project.xml file :

````
<use project = "fty-common-rest" libname = "libfty_common_rest" header="fty_common_rest.h"
    repository = "https://github.com/42ity/fty-common-rest.git"
    test = "fty_commmon_rest_selftest" >
    <use project = "libsodium" prefix = "sodium"
        repository = "https://github.com/42ity/libsodium.git"
        release = "1.0.5-FTY-master"
        test = "sodium_init"
        />
    <use project = "cxxtools"
        test="cxxtools::Utf8Codec::Utf8Codec"
        header = "cxxtools/allocator.h"
        repository = "https://github.com/42ity/cxxtools.git"
        release = "2.2-FTY-master"
        />
    <use project = "czmq"
        repository="https://github.com/42ity/czmq.git"
        release = "v3.0.2-FTY-master"
        min_major = "3" min_minor = "0" min_patch = "2" >
        <use project = "libzmq"
            repository="https://github.com/42ity/libzmq.git"
            release = "4.2.0-FTY-master" >
            <use project = "libsodium" prefix = "sodium"
                repository = "https://github.com/42ity/libsodium.git"
                release = "1.0.5-FTY-master"
                test = "sodium_init"
                />
        </use>
    </use>
    <use project = "tntnet"
        header = "tnt/tntnet.h"
        repository = "https://github.com/42ity/tntnet.git"
        release = "2.2-FTY-master"
        />
    <use project = "libsasl2" header = "sasl/sasl.h"
        debian_name="libsasl2-dev"
        redhat_name="cyrus-sasl-devel"
    />
    <use project = "fty-common-logging" libname = "libfty_common_logging" header="fty_log.h"
        repository = "https://github.com/42ity/fty-common-logging.git"
        release = "master"
        test = "fty_common_logging_selftest" >
        <use project = "log4cplus" header = "log4cplus/logger.h" test = "appender_test"
            repository = "https://github.com/42ity/log4cplus.git"
            release = "1.1.2-FTY-master"
            />
    </use>
    <use project = "fty-common" libname = "libfty_common" header="fty_common.h"
        repository = "https://github.com/42ity/fty-common.git"
        release = "master" >
        <use project = "fty-common-logging" libname = "libfty_common_logging" header="fty_log.h"
            repository = "https://github.com/42ity/fty-common-logging.git"
            release = "master"
            test = "fty_common_logging_selftest" >
            <use project = "log4cplus" header = "log4cplus/logger.h" test = "appender_test"
                repository = "https://github.com/42ity/log4cplus.git"
                release = "1.1.2-FTY-master"
                />
        </use>
    </use>
    <use project = "fty-common-db" libname = "libfty_common_db" header="fty_common_db.h"
        repository = "https://github.com/42ity/fty-common-db.git"
        release = "master" >
        <use project = "czmq"
            repository="https://github.com/42ity/czmq.git"
            release = "v3.0.2-FTY-master"
            min_major = "3" min_minor = "0" min_patch = "2" >
            <use project = "libzmq"
                repository="https://github.com/42ity/libzmq.git"
                release = "4.2.0-FTY-master" >
                <use project = "libsodium" prefix = "sodium"
                    repository = "https://github.com/42ity/libsodium.git"
                    release = "1.0.5-FTY-master"
                    test = "sodium_init" />
            </use>
        </use>
        <use project = "tntdb"
            test="tntdb::Date::gmtime"
            repository = "https://github.com/42ity/tntdb.git"
            release = "1.3-FTY-master"
            builddir = "tntdb">
            <use project = "cxxtools" test="cxxtools::Utf8Codec::Utf8Codec" header="cxxtools/allocator.h"
                repository = "https://github.com/42ity/cxxtools.git"
                release = "2.2-FTY-master"
                />
        </use>
        <use project = "fty-common" libname = "libfty_common" header="fty_common.h"
            repository = "https://github.com/42ity/fty-common.git"
            release = "master" >
            <use project = "fty-common-logging" libname = "libfty_common_logging" header="fty_log.h"
                repository = "https://github.com/42ity/fty-common-logging.git"
                release = "master"
                test = "fty_common_logging_selftest" >
                <use project = "log4cplus" header = "log4cplus/logger.h" test = "appender_test"
                    repository = "https://github.com/42ity/log4cplus.git"
                    release = "1.1.2-FTY-master"
                    />
            </use>
        </use>
    </use>
</use>
````
