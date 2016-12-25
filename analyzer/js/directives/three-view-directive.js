angular.module('high-start').directive('threeView', function(){
    
  return {
    restrict: 'E',
      replace: true,
    scope: {
      title: '@'
    },
    link: function(scope, element, attrs, tabsCtrl) {
            var scene = new THREE.Scene();

            var camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1000);

            var webGLRenderer = new THREE.WebGLRenderer();
            webGLRenderer.setSize(window.innerWidth, window.innerHeight);
            webGLRenderer.shadowMapEnabled = true;

            
            var from = new THREE.Vector3(2, 2, 2);
            var to = new THREE.Vector3(30, 30, 30);
            var direction = to.clone().sub(from);
            var length = direction.length();
            var arrowHelper = new THREE.ArrowHelper(direction.normalize(), from, length, 0xff0000);
            scene.add(arrowHelper);


            camera.position.x = -30;
            camera.position.y = 40;
            camera.position.z = 50;
            camera.lookAt(new THREE.Vector3(10, 0, 0));

            element[0].appendChild(webGLRenderer.domElement);

            var step = 0;

            render();

            function render() {

                arrowHelper.rotation.y = step += 0.01;

                // render using requestAnimationFrame
                requestAnimationFrame(render);
                webGLRenderer.render(scene, camera);
            }
    },
    template: '<div></div>'
  };    
    
});