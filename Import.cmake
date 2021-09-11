### 
# Import common scripts for setup build and testing 
###
function(IMPORT_BUILD_SCRIPTS)
    include(FetchContent)
    FetchContent_Declare(
        build_scripts
        GIT_REPOSITORY https://github.com/JekasObps/build-scripts.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(build_scripts)
endfunction(IMPORT_BUILD_SCRIPTS)


