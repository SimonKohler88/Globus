import sys

from vispy import scene
from vispy.visuals.transforms import STTransform

canvas = scene.SceneCanvas(keys='interactive', bgcolor='white',
                           size=(800, 600), show=True)

view = canvas.central_widget.add_view()
view.camera = 'arcball'

sphere = scene.visuals.Sphere(radius=1, rows=10, cols=10, depth=10,
                               method='cube', parent=view.scene,
                               shading='flat')



view.camera.set_range(x=[-3, 3])

if __name__ == '__main__' and sys.flags.interactive == 0:
    canvas.app.run()