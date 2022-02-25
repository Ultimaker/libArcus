from sipbuild import SetuptoolsBuilder


class CMakeBuilder(SetuptoolsBuilder):
    def __init__(self, project, **kwargs):
        print("Using the CMake builder")
        super().__init__(project, **kwargs)

    def build(self):
        """ Only Generate the source files """
        print("Generating the source files")
        self._generate_bindings()
        self._generate_scripts()