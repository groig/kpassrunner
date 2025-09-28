cmake_minimum_required(VERSION 3.16)
project(passrunner)

set(KF6_MIN_VERSION 6.0.0)
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS I18n Runner Notifications)

kcoreaddons_add_plugin(passrunner
    SOURCES passrunner.cpp
    INSTALL_NAMESPACE "kf6/krunner"
)

target_link_libraries(passrunner
    KF6::I18n
    KF6::Runner
    KF6::Notifications
)

install(FILES passrunner.notifyrc
        DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
