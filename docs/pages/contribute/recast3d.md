# RECAST3D

RECAST3D is based on TomoPackets (ZeroMQ) for communication and OpenGL for visualization. It runs a single _server_, which communicates with a reconstruction server such as SliceRecon.

First, a brief summary of the RECAST3D design. The slicer environment is implemented in `ReconstructionComponent`, but there is much more in RECAST3D. It has a flexible and transparent server that distributes TomoPackets to modules that want to know about them. Using graphics components and modules, it is easy to extend, improve, or change behaviour. The rendering is based on OpenGL, making it flexible but not trivial to extend. I provide texture, shader, meshes and primitive support, which makes it easier to make a new component. The immediate mode user interface is straightforward to use.


## Graphics and scenes

The software can have multiple scenes. Each scene corresponds to the visualization of a single scan.

The important classes that realize the graphics are organized as follows:

<div class="mermaid">
    classDiagram
      class AxesComponent
      class GeometryComponent
      class MeshComponent
      class ReconstructionComponent
      class Component {
          identifier
          draw()
          tick()
          describe()
      }
      class SceneObject {
        camera
        graphics resources
      }
      class Scene {
        name
        dimension
        data
      }


      AxesComponent <|-- Component
      GeometryComponent <|-- Component
      MeshComponent <|-- Component
      ReconstructionComponent <|-- Component

      SceneObject <|-- Scene : Contains
      Component <|-- SceneObject : Contains
</div>

The graphics are rendered directly using OpenGL, but there are helper classes available (for primitives, shader programs, textures, and so on).

Important shared interfaces for many classes in the graphics implementations are:

<div class="mermaid">
classDiagram
  class RenderTarget {
     render(...) 
  }
  class Ticker {
     tick(time_elapsed) 
  }
  class Window {
     describe()
  }
  class PacketPublisher {
    send(packet)
    add_listener(...)
  }
  class PacketListener {
    handle(packet)
  }
  PacketPublisher <|-- PacketListener
</div>

- `RenderTarget` is for any component that wants to be rendered, such as a scene or the user interface. A scene eventually calls `draw` on its components which takes into account the (3D) camera.
- `Ticker` is implemented by any object that wants to update itself over regular time intervals.
- `Window` is implemented by any object that wants to add controls to the user interface
- `PacketPublisher` and `PacketListener` allows to easily send packets to upstream components of the reconstruction stack from any point in the graphics (for example, as a side effect due to user input).

## UI elements

We use [ImGui](https://github.com/ocornut/imgui/tree/master/docs) for the user interface. This allows parts of the graphics stack to easily draw check boxes, sliders, and text input. This is done through the `Window` interface.

There is also built in support for parameters of upstream components, using the `ParameterX` packets of TomoPackets.

## Server

The server has a number of 'modules'. Each module corresponds to a part of RECAST3D that is interested in handling certain packages. A selection of the implemented modules:

<div class="mermaid">
classDiagram
  class SceneModuleProtocol {
     read_packet(...) 
     process(...)
     descriptors(...)
  }

   ManageSceneProtocol <|-- SceneModuleProtocol
   ReconstructionProtocol <|-- SceneModuleProtocol
   GeometryProtocol <|-- SceneModuleProtocol
   ControlProtocol <|-- SceneModuleProtocol
</div>

For example, the manage scene protocol listens only to incoming packets of type `MakeScenePacket`, and whenever one comes in it adds a scene to the scene list. Something to note is that `read_packet` happens on the server thread, while `process` happens on the graphics thead. These are kept separate so that all OpenGL calls happen from within a single thread.

There is a server running the ZeroMQ `REP/REQ` protocol that handles incoming packets from upstream (by default on port 5555), and a thread running a `PUB/SUB` protocol that publishes outgoing packets to (multiple) listeners (by default on port 5556).

## _Example_: Reconstruction of a re-oriented slice

Let us explore in detail what happens when the user changes a slice. This happens when the user translates along the normal of a slice (using the left mouse button), or rotates around the furthest edge from where they click (using the right button).

`ReconstructionComponent` optionally has a `ReconDragMachine` which is initiated as soon as the user clicks on the mouse. There are two kinds: a `SliceTranslator` and a `SliceRotator`, which both have an `on_drag` event.

<div class="mermaid">
stateDiagram
    Idle --> Translating : left_mouse_down
    Translating --> Idle : left_mouse_up
    state Translating {
      [*] --> [*] : mouse_drag
    }
    Idle --> Rotating : right_mouse_down
    Rotating --> Idle : right_mouse_up
    state Rotating {
      [*] --> [*] : mouse_drag
    }
</div>

These machines actually deactivate a slice upon creation, and have a reference to a 'ghost' slice that is will be created when the mouse is released.

When this new slice is created, we have to let the reconstruction server now that this has occured. A `SetSlicePacket` is created using the orientation of the newly created slice, and published using the `PUB/SUB` servers to upstream clients that are subscribed to this packets. For example, SliceRecon registers to this packet.

<div class="mermaid">
sequenceDiagram
    participant Component
    participant SliceRecon
    participant Protocol
    Component->>SliceRecon: SetSlicePacket
    SliceRecon->>Protocol: SliceDataPacket
    Protocol->>Component: set_data(...)
</div>

The reconstruction server then computes a slice reconstruction for the slice, and sends this to the incoming port of RECAST3D (the `REQ/REP` server). This is eventually handled by the `ReconstructionProtocol` that listens to `SliceDataPackets`. When this protocol handles the request, it updates the data of the corresponding scene, and this is ultimately reflected  in the ReconstructionComponent which then shows the updated slice.

Note that this entire sequence happens within milliseconds, and is entirely distributed: RECAST3D and the reconstruction server do not have to run on the same computer, and in fact any agent can act as the reconstruction server.

## _Example_: Extend RECAST3D to add control parameters

As an example of a feature that was added to RECAST3D, let's discuss the steps it took to add control parameters to RECAST3D. The idea of control parameters is that upstream components can register a 'parameter' with RECAST3D, which then shows a UI element to change this parameter. When the user changes one of these parameters using the UI, a packet has to be sent to the upstream component that it belongs to.

1. Control packets were added to TomoPackets (e.g. `ParameterBoolPacket`)
2. A `ControlComponent` and a `ControlModule` were added to RECAST3D. Note that the control component does not draw anything in the 3D window, but only wants to add additional UI elements.
3. The `ControlModule` registers itself as the handler for e.g. the `ParameterBoolPacket` and others (actual implementation also includes benchmarking, and tracking).
4. When a `ParameterBoolPacket` is received by the module it gets sent to the appropriate `ControlComponent`. 
5. The implementation of the `ControlComponent`:
    - In `describe()`, for drawing UI elements: one checkbox for each known parameter.
    - If checkbox changed by user, we send a `ParameterBoolPacket` downstream.

After these changes, we can use this for real-time alignment parameter adjustment, changing filters, and so on, all without touching existing code in RECAST3D.
