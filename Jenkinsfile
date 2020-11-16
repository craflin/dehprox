pipeline {
    agent none
    stages {
        stage('All') {
            matrix {
                agent {
                    label "${platform}"
                }
                axes {
                    axis {
                        name 'platform'
                        values 'ubuntu20.04-x86_64', 'ubuntu18.04-x86_64', 'raspbian10-armv6l'
                    }
                }
                stages {
                    stage('Build') {
                        steps {
                            cmakeBuild buildDir: 'build', cleanBuild: true, installation: 'InSearchPath', buildType: 'Release', cmakeArgs: '-G Ninja'
                            cmake workingDir: 'build', arguments: '--build . --target package', installation: 'InSearchPath'
                        }
                    }
                }
                post { 
                    always {
                        archiveArtifacts artifacts: 'build/dehprox-*.deb'
                    }
                } 
            }
        }
    }
}

