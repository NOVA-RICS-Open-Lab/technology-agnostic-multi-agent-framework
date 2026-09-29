import os
import sys
import subprocess

def main():
    os.environ["DEFAULT_EMA_HEARTBEAT_INTERVAL"] = "60000"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    bin_dir = os.path.join(script_dir, "bin")
    java_pkg_classes = os.path.abspath(os.path.join(script_dir, "..", "..", "mk3", "JavaPackage", "target", "classes"))
    
    m2_repo = os.path.expanduser("~/.m2/repository")
    jackson_core = os.path.join(m2_repo, "com", "fasterxml", "jackson", "core", "jackson-core", "2.15.2", "jackson-core-2.15.2.jar")
    jackson_databind = os.path.join(m2_repo, "com", "fasterxml", "jackson", "core", "jackson-databind", "2.15.2", "jackson-databind-2.15.2.jar")
    jackson_annotations = os.path.join(m2_repo, "com", "fasterxml", "jackson", "core", "jackson-annotations", "2.15.2", "jackson-annotations-2.15.2.jar")
    jackson_jsr310 = os.path.join(m2_repo, "com", "fasterxml", "jackson", "datatype", "jackson-datatype-jsr310", "2.15.2", "jackson-datatype-jsr310-2.15.2.jar")
    
    classpath = os.pathsep.join([bin_dir, java_pkg_classes, jackson_databind, jackson_core, jackson_annotations, jackson_jsr310])
    
    cmd = ["java", "-cp", classpath, "tamaf.stresstest.Main"] + sys.argv[1:]
    
    result = subprocess.run(cmd, cwd=script_dir)
    sys.exit(result.returncode)

if __name__ == "__main__":
    main()
