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
