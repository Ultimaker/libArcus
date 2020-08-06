parallel_nodes(["linux && cura", "windows && cura"]) {
    stage('Prepare') {
        step([$class: 'WsCleanup'])

        checkout scm
    }

    catchError {
        dir('build') {
            stage('Build') {
                def branch = env.BRANCH_NAME
                if(!fileExists("${env.CURA_ENVIRONMENT_PATH}/${branch}")) {
                    branch = "master"
                }

                def extra_cmake_options = ""
                if(!isUnix())
                {
                    // Disable building of Python bindings on Windows for now,
                    // since it requires MSVC to build properly.
                    extra_cmake_options = "-DBUILD_PYTHON=OFF"

                    // Building example requires dynamic linking, while with MingGW
                    // on Windows in the build environment doesn't have dynamic protobuf
                    // library, so we can't do dynamic linking.
                    extra_cmake_options = "${extra_cmake_options} -DBUILD_EXAMPLES=OFF"
                    extra_cmake_options = "${extra_cmake_options} -DBUILD_STATIC=ON"

                    // The CI is using MinGW as the CMake target, but the MinGW protobuf
                    // libraries are installed in "lib-mingw" instead of the default "lib"
                    // directory. We need to specify to use "lib-mingw" specifically so
                    // the linking process can use the correct library.
                    extra_cmake_options = "${extra_cmake_options} -DPROTOBUF_LIBRARY=\"${env.CURA_ENVIRONMENT_PATH}/${branch}/inst/lib-mingw/libprotobuf.a\""
                }

                cmake '..', "-DCMAKE_PREFIX_PATH=\"${env.CURA_ENVIRONMENT_PATH}/${branch}\" -DCMAKE_BUILD_TYPE=Release ${extra_cmake_options}"
                make('')
            }
        }
    }

    stage('Finalize') {
        notify_build_result(env.CURA_EMAIL_RECIPIENTS, '#cura-dev', ['master'])
    }
}
