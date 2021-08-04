#include <vector>
#include <iostream>
#include <thread>

#include "Arcus/Socket.h"
#include "Arcus/SocketListener.h"
#include "Arcus/Error.h"

#include "test.pb.h"

struct Object
        {
        public:
            int id;
            std::string vertices;
            std::string normals;
            std::string indices;
        };

class Listener : public Arcus::SocketListener
        {
        public:
            void stateChanged(Arcus::SocketState::SocketState new_state)
            {
                std::cout << "State Changed: " << new_state << std::endl;
            }

            void messageReceived()
            {
            }

            void error(const Arcus::Error& new_error)
            {
                std::cout << new_error << std::endl;
            }
        };

std::vector<Object> objects;

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message);

int main(int argc, char** argv)
{
    Arcus::Socket socket;

    socket.registerMessageType(&Example::ObjectList::default_instance());
    socket.registerMessageType(&Example::ProgressUpdate::default_instance());
    socket.registerMessageType(&Example::SlicedObjectList::default_instance());

    return 0;
}

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message)
{
    // (Dynamicly) cast the message to one of our types. If this works (does not return a nullptr), we've found the right type.
    auto objectList = dynamic_cast<Example::ObjectList*>(message.get());
    if(objectList)
    {
        objects.clear();

        std::cout << "Received object list containing " << objectList->objects_size() << " objects" << std::endl;

        for(auto objectDesc : objectList->objects())
        {
            Object obj;
            obj.id = objectDesc.id();
            obj.vertices = objectDesc.vertices();
            obj.normals = objectDesc.normals();
            obj.indices = objectDesc.indices();
            objects.push_back(obj);
        }

        auto msg = std::make_shared<Example::SlicedObjectList>();
        int progress = 0;
        for(auto object : objects)
        {
            auto slicedObject = msg->add_objects();
            slicedObject->set_id(object.id);

            for(int i = 0; i < 1000; ++i)
            {
                auto polygon = slicedObject->add_polygons();
                polygon->set_type(i % 2 == 0 ? Example::Polygon_Type_InnerType : Example::Polygon_Type_OuterType);
                polygon->set_points(object.vertices);
            }

            auto update = std::make_shared<Example::ProgressUpdate>();
            update->set_objectid(object.id);
            update->set_amount((float(++progress) / float(objects.size())) * 100.f);
            socket.sendMessage(update);
        }

        std::cout << "Sending SlicedObjectList" << std::endl;
        socket.sendMessage(msg);

        return;
    }
}
