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

###
#   Import git repository 
###
function(IMPORT_REPO name git_url git_tag)
include(FetchContent)
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${git_url}
        GIT_TAG ${git_tag}
    )

    FetchContent_MakeAvailable(${name})
endfunction(IMPORT_REPO)
