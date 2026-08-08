(load "~/redbuild/v3/redbuild.lisp")
(load "~/redbuild/packaging/packager.lisp")

(redbuild:set-global-target (redbuild:native))

(redbuild:make "~/redlib" "cross")

(defvar *beyond-lib* t)
(defvar *beyond-interpreter* t)
(redbuild:build-dep "~/beyond")

(redbuild:set-tester "examples/scripting.c")

(redbuild:quick-build (redbuild:make-instance `redbuild:redmod
        :name "three2d"
        :type (redbuild:dynlib)
        :target (redbuild:dyn-target)
        :libs (list (redbuild:local-lib "beyond" :lib "imaginal.a"))
        :srcs (redbuild:dynsrc "obj.c" "render.c" "shaders.c" "mesh.c" "geolisp/geolisp.c")
        :flags (list "-g")
) :add-dependencies t :run t :debug-symbols t :success (lambda () 
    (redbuild:emit-compile-commands)
))
