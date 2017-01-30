node("linux && cura") {
    stage('Prepare') {
        step([$class: 'WsCleanup'])

        checkout scm
    }

    catchError {
        dir('build') {
            stage('Build') {
                sh 'cmake .. -DCMAKE_PREFIX_PATH=/opt/ultimaker/cura-build-environment -DCMAKE_BUILD_TYPE=Release'
                sh 'make'
            }
        }
    }

    stage('Finalize') {
        if(currentBuild.result && currentBuild.result != "SUCCESS")
        {
            if(currentBuild.previousBuild.result != currentBuild.result)
            {
                emailext(
                    subject: "[Jenkins] Build ${currentBuild.fullDisplayName} has become ${currentBuild.result}",
                    body: "Jenkins build ${currentBuild.fullDisplayName} changed from ${currentBuild.previousBuild.result} to ${currentBuild.result}.\n\nPlease check the build output at ${env.BUILD_URL} for details.",
                    to: env.CURA_EMAIL_RECIPIENTS
                )
            }
            else
            {
                emailext (
                    subject: "[Jenkins] Build ${currentBuild.fullDisplayName} is ${currentBuild.result}",
                    body: "Jenkins build ${currentBuild.fullDisplayName} is ${currentBuild.result}\n\nPlease check the build output at ${env.BUILD_URL} for details.",
                    to: env.CURA_EMAIL_RECIPIENTS
                )
            }
        }
        else
        {
            if(currentBuild.previousBuild.result != currentBuild.result)
            {
                emailext(
                    subject: "[Jenkins] Build ${currentBuild.fullDisplayName} was fixed!",
                    body: "Jenkins build ${currentBuild.fullDisplayName} was ${currentBuild.previousBuild.result} but is now stable again.\n\nPlease check the build output at ${env.BUILD_URL} for details.",
                    to: env.CURA_EMAIL_RECIPIENTS
                )
            }
        }
    }
}
