# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler;C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/build")
set(CPACK_CMAKE_GENERATOR "MinGW Makefiles")
set(CPACK_COMPONENTS_ALL "Applications;Documentation;Examples")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Main executable and required libraries")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Tocin Compiler Application")
set(CPACK_COMPONENT_APPLICATIONS_REQUIRED "TRUE")
set(CPACK_COMPONENT_DOCUMENTATION_DEPENDS "applications")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Documentation files")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS "applications")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example Tocin programs")
set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Example Programs")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "C:/msys64/mingw64/share/cmake/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "Tocin built using CMake")
set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE "ON")
set(CPACK_GENERATOR "7Z;ZIP")
set(CPACK_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALLED_DIRECTORIES "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler;/")
set(CPACK_INSTALL_CMAKE_PROJECTS "")
set(CPACK_INSTALL_PREFIX "C:/Program Files (x86)/Tocin")
set(CPACK_MODULE_PATH "C:/msys64/mingw64/lib/cmake/llvm")
set(CPACK_NSIS_DISPLAY_NAME "Tocin Compiler")
set(CPACK_NSIS_DISPLAY_NAME_SET "TRUE")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "SetShellVarContext all
                     # Request admin privileges
     SetOutPath "$INSTDIR\bin"

     CreateDirectory "$SMPROGRAMS\tocin"

     CreateShortCut "$SMPROGRAMS\tocin\tocin.lnk" "$INSTDIR\bin\tocin.exe"

     CreateShortCut "$DESKTOP\tocin.lnk" "$INSTDIR\bin\tocin.exe"

     WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" "$INSTDIR\bin;$PATH"

     System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    # Notify Windows of the PATH change
     SetRegView 64
                                                        # For 64-bit registry view
     WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TOCIN_HOME" "$INSTDIR"
    ")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "SetShellVarContext all
                      # Request admin privileges
     Delete "$DESKTOP\tocin.lnk"

     RMDir /r "$SMPROGRAMS\tocin"

     DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TOCIN_HOME"

      $0 "PATH" "R" "HKLM" "$INSTDIR\bin"
       # Remove from PATH
     System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    # Notify Windows of the PATH change
    ")
set(CPACK_NSIS_HELP_LINK "https://tocin-support.vercel.app/support")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\tocin.exe")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "bin\\tocin.exe")
set(CPACK_NSIS_MUI_ICON "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/Tocin_Logo.ico")
set(CPACK_NSIS_MUI_UNIICON "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/Tocin_Logo.ico")
set(CPACK_NSIS_PACKAGE_NAME "Tocin Compiler")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_NSIS_URL_INFO_ABOUT "https://tocin-support.vercel.app")
set(CPACK_OBJCOPY_EXECUTABLE "C:/msys64/mingw64/bin/objcopy.exe")
set(CPACK_OBJDUMP_EXECUTABLE "C:/msys64/mingw64/bin/objdump.exe")
set(CPACK_OUTPUT_CONFIG_FILE "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/build/CPackConfig.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "C:/msys64/mingw64/share/cmake/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tocin Compiler")
set(CPACK_PACKAGE_FILE_NAME "tocin-1.0.0-Source")
set(CPACK_PACKAGE_ICON "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler\\Tocin_Logo.ico")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ToCin")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ToCin")
set(CPACK_PACKAGE_NAME "tocin")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "Afolabi_Oluwatosin")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_READELF_EXECUTABLE "C:/msys64/mingw64/bin/readelf.exe")
set(CPACK_RESOURCE_FILE_LICENSE "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/LICENSE")
set(CPACK_RESOURCE_FILE_README "C:/msys64/mingw64/share/cmake/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "C:/msys64/mingw64/share/cmake/Templates/CPack.GenericWelcome.txt")
set(CPACK_RPM_PACKAGE_SOURCES "ON")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "ON")
set(CPACK_SOURCE_GENERATOR "7Z;ZIP")
set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler;/")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/build/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "tocin-1.0.0-Source")
set(CPACK_SOURCE_TOPLEVEL_TAG "win64-Source")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_STRIP_FILES "")
set(CPACK_SYSTEM_NAME "win64")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "win64-Source")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "C:/Users/Afolabi Oluwatoisn A/Downloads/tocin-compiler/tocin-compiler/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
