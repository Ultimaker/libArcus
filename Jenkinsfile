node("linux && cura") {
    stage('Prepare') {
        step([$class: 'WsCleanup'])

        checkout scm
    }

    catchError {
        dir('build') {
            stage('Build') {
                def branch = env.BRANCH_NAME
                if(!(branch =~ /^2.\d+$/)) {
                    branch = "master"
                }

                sh 'cmake .. -DCMAKE_PREFIX_PATH=/opt/ultimaker/cura-build-environment/${branch} -DCMAKE_BUILD_TYPE=Release'
                sh 'make'
            }
        }
    }

    stage('Finalize') {
        notify_build_result(env.CURA_EMAIL_RECIPIENTS, '#cura-dev', ['master'])
    }
}
