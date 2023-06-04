#include "NavMesh.h"
#include <fstream>
#ifdef EDITOR
#include "../Engine/src/Graphics/DebugDraw/DebugDrawing.h"
#include "Physics/Physics.h"
#include "Utilities/Input/Input.h"
#include "../Editor/src/Editor.h"
#include "../Editor/src/BasicFunctionalities/ObjectSelection/MousePicker/MousePicker.h"
#endif // EDITOR

void NavMesh::Initialize()
{
    LoadFromTXT("./../Resources/toLoadNavMesh.TXT");
    
    srcLine = -1; //not nodes selected yer
    dstLine = -1;

    showPath = false;
    toDelete = -1;
    pointFollowing = { 0.0f,0.0f,0.0f };
}

void NavMesh::Update()
{
}

void NavMesh::Shutdown()
{
}

#ifdef EDITOR
bool NavMesh::Edit()
{
    //set positions

    ImGui::Text("TABLE...");

    if (ImGui::CollapsingHeader("Columns"))
        EditColumns();

    ShowNodes();

    if (ImGui::CollapsingHeader("Algorithm Parameters"))
        RunAlgEdit();

    //show object following shortest path
    if (showPath)
        ShowPathFollowing();

	return true;

}

bool NavMesh::ConnectNodes()
{
    return false;
}


void NavMesh::DeleteNode(unsigned nodeID)
{
    auto& theTable = mGraph.GetCurrentTable();
    auto& positions = mGraph.GetPosArray();

    unsigned index = nodeID - 1u;

    //Erase all edges of node to delete 
    theTable.erase(theTable.begin() + index);
    positions.erase(positions.begin() + index);

    auto size = positions.size();
    //ERASE THE PLACE CORRESPONDOING TO THIS ID ON EVERY PART OF THE TABLE
    for (unsigned i = 0u; i < size; i++) //iterate through all nodes 
    {
        //erase the current node
        auto& Row = theTable[i];
        Row.erase(Row.begin() + index);
    }

    //erase a value from the table
    auto& dTable = mGraph.getDijkstraTable();
    dTable.erase(dTable.begin() + index);
    SaveToTXT("./../Resources/toLoadNavMesh.TXT");
    mGraphSize--;
    mGraph.mSize = mGraphSize;
}




bool NavMesh::ShowNodes()
{
    auto& theTable = mGraph.GetCurrentTable();
    auto& positions = mGraph.GetPosArray();

    //SHOW NODES 
    for (unsigned i = 0u; i < theTable.size(); i++)
    {
        glm::vec3 pos = positions[i];

        geometry::sphere s; 
        s.mCenter = pos;
        s.mRadius = 2.5f;

        Debug::DrawSphere(s, { 0.0f,1.0f,0.0f,1.0f });

        //SHOW CONNECTIONS
        for (unsigned j = 0u; j < theTable.size(); j++)
        {
            glm::vec3 pos2 = positions[j];
            geometry::segment seg;
            seg.p1 = pos;
            seg.p2 = pos2;
            
            //only draw if the cost is not infinite
            unsigned cost = theTable[i][j].weight;

            if(cost < 999 && !showPath)
                Debug::DrawLine(seg, { 0.0f,1.0f,0.0f,1.0f });
        }

    }

    //DRAG FROM A NODE TO ANOTHER TO CONNECT
    if (InputManager.KeyIsTriggered(Key::Control))
    {
        unsigned nodeIdx = GetNodeInMouse();
        
        if (nodeIdx < -1) //found node on mouse
            srcLine = nodeIdx;
    }

    if (InputManager.KeyIsReleased(Key::Control) && srcLine < -1)
    {
        unsigned nodeIdx = GetNodeInMouse();

        if (nodeIdx < -1 && nodeIdx != srcLine) { //found node on mouse
            dstLine = nodeIdx;

            //set edge
            mGraph.LinkNodes(srcLine, dstLine);
        }

        srcLine = -1;
        dstLine = -1;
    }


    return true;
}
bool NavMesh::RunAlgEdit()
{
    int* nasty = reinterpret_cast<int*>(&mGraph.startNode);
    int* lastNasty = reinterpret_cast<int*>(&mGraph.endNode); //node where we want to go
    auto& positions = mGraph.GetPosArray();

    ImGui::DragInt("Start Node: ", nasty, 0.4f, 1, (int)mGraph.GetCurrentTable().size());
    ImGui::DragInt("End Node", lastNasty, 0.4f, 1, (int)mGraph.GetCurrentTable().size());

    if (ImGui::Button("run dijkstra"))
    {
        mGraph.Dijkstra(mGraph.startNode);
        SaveToTXT("./../Resources/toLoadNavMesh.TXT");
    }

    bool previousPath = showPath;
    ImGui::Checkbox("Show Shortest paths", &showPath);

    auto& result = mGraph.shortestPaths();
    auto& rpath = result[mGraph.endNode - 1];
    auto& posarray = mGraph.GetPosArray(); //postion
    //show the algorithm result
    if (showPath && !rpath.path.empty())
    {

        //show the demo if just activated
        if (!previousPath)
        {
            mDemoPath = rpath.path;
            pointFollowing = posarray[rpath.path[0] -1u];
        }

        for (unsigned i = 0u; i < rpath.path.size() - 1u; i++) //draw line of path
        {
            unsigned v1 = rpath.path[i]; //node 1
            unsigned v2 = rpath.path[i+1];//node 2

            geometry::segment toDraw; 
            toDraw.p1 = posarray[v1 - 1u]; //postion
            toDraw.p2 = posarray[v2 - 1u]; //postion

            Debug::DrawLine(toDraw, { 1.0f,0.0f,0.0f,1.0f });
        }
    }

    return true;
}
bool NavMesh::EditColumns()
{
    //delete in the next frame we put delete
    if (toDelete < -1)
    {
        DeleteNode(toDelete + 1u);
        toDelete = -1;
    }

    ImGui::PushID("Columns");

    static bool disable_indent = false;
    if (disable_indent)
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);

    // NB: Future columns API should allow automatic horizontal borders.
    auto& theTable = mGraph.GetCurrentTable();
    auto& positions = mGraph.GetPosArray();

    bool h_borders = true;
    bool v_borders = true;
    const int columns_count = (int)theTable.size();
    const int lines_count = (int)theTable.size();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
    ImGui::Columns(columns_count + 1, nullptr, v_borders);

    unsigned rowCount = columns_count;

    for (int i = 0; i < columns_count + 1; i++)
    {
        std::string number = std::to_string(i);
        ImGui::Text(number.c_str());
        ImGui::NextColumn();
    }
    for (unsigned i = 0u; i < rowCount; i++)
    {
        std::string number = std::to_string(i + 1);
        ImGui::Text(number.c_str());
        ImGui::SameLine();
        ImGui::PushID(i);
        ImGui::DragFloat3("pos: ", &positions[i][0]);
        ImGui::SameLine();

        if (ImGui::Button("Delete"))
            toDelete = i;

        ImGui::PopID();
        ImGui::NextColumn();

        for (int j = 0u; j < columns_count; j++)
        {
            if (theTable[i].size() != 0u && j < theTable[i].size())
            {
                int* nasty = reinterpret_cast<int*>(&theTable[i][j].weight);
                ImGui::PushID(nasty);
                ImGui::DragInt("", nasty);
                ImGui::PopID();

            }
            else
            {
                int temp = 9999;
                ImGui::PushID(&temp);
                ImGui::DragInt("", &temp);
                ImGui::PopID();
            }
            ImGui::NextColumn();
        }
    }

    ImGui::Columns(1);
    
    if (h_borders)
        ImGui::Separator();

    if (disable_indent)
        ImGui::PopStyleVar();
    ImGui::PopID();

    if (ImGui::Button("Insert Node"))
        mGraph.InsertNode();

    return true;
}
bool NavMesh::ShowPathFollowing()
{
    //we gone through all nodes or the demo is empty
    if (mDemoPath.empty()) return false;
    if (mDemoPath.size() <= 1) return false;

    unsigned currentID = mDemoPath.front() -1;
    unsigned nextID = mDemoPath[1] - 1;
    auto& posArray = mGraph.GetPosArray();
    glm::vec3 currentPoint = posArray[currentID];
    glm::vec3 NextPoint = posArray[nextID];
    glm::vec3 followVector = NextPoint - currentPoint;
    followVector = glm::normalize(followVector);
    followVector /= 10.0f;

    //GET FOLLOWING
    pointFollowing += followVector;
    geometry::sphere s;
    s.mRadius = 1.5f;
    s.mCenter = pointFollowing;

    geometry::sphere sToFollow;
    sToFollow.mRadius = 2.5f;
    sToFollow.mCenter = NextPoint;

    Debug::DrawSphere(s, { 0.0f,1.0f,0.0f,1.0f });
    
    s.mRadius = 0.2f;
    //When we reach the next point, we pop the point
    if (CollisionManager.SpherevsSphere(s, sToFollow))
        mDemoPath.erase(mDemoPath.begin());

    return false;
}
unsigned NavMesh::GetNodeInMouse()
{
    MousePicker m{ &EditorMgr.mCamera }; //change this later
    glm::vec2 mousePos;
    geometry::ray r;
    mousePos.x = static_cast<float>(InputManager.RawMousePos().x);
    mousePos.y = static_cast<float>(InputManager.RawMousePos().y);

    auto& nodePositions = mGraph.GetPosArray();

    for (unsigned i = 0u; i < nodePositions.size(); i++)
    {
        r.mDirection = glm::vec3(m.ScreenToWorld(mousePos));
        r.mOrigin = EditorMgr.mCamera.GetPosition();

        geometry::sphere s;
        s.mRadius = 2.5f;
        s.mCenter = nodePositions[i];
        float result = geometry::intersection_ray_sphere(r, s);
        
        if (result > 0.0f)
            return  i;
    }
    return -1;
}
#endif // EDITOR

std::vector<glm::vec3> NavMesh::GetClosetPath(unsigned startNode, unsigned endNode, std::vector<unsigned>* pathToFill, unsigned* TotalCost)
{
    mGraph.Dijkstra(startNode);
    std::vector<glm::vec3> toRetrun(0u);

    if (endNode == -1) endNode = 1; //Sanity check 

    auto& table = mGraph.getDijkstraTable()[endNode - 1];
    
   // if (TotalCost) (*TotalCost) = table.cost;

    auto& pathIndices = mGraph.getDijkstraTable()[endNode - 1].path;

    for (auto& it : pathIndices) {
        unsigned posIndex = it;
        glm::vec3 pos = mGraph.GetPosArray()[posIndex -1];
        toRetrun.push_back(pos);

        if (pathToFill)
            pathToFill->push_back(posIndex);
    }

    return toRetrun;
}

IComp* NavMesh::Clone()
{
	return mOwner->GetSpace()->CreateComp<NavMesh>(this);
}

void NavMesh::ToJson(nlohmann::json& j) const
{
}

void NavMesh::FromJson(nlohmann::json& j)
{
}

void NavMesh::LoadFromTXT(std::string path)
{
    std::ifstream in(path);

    in >> mGraphSize;
    in.ignore();

    mGraph.Create(mGraphSize);
    auto& posArray = mGraph.GetPosArray();

    //get points
    for (unsigned i = 0; i < mGraphSize; i++)
    {
        in >> posArray[i].x;
        in.ignore();
        in >> posArray[i].y;
        in.ignore();
        in >> posArray[i].z;
        in.ignore();
    }

    //get matrix
    auto& table = mGraph.GetCurrentTable();
    for (unsigned i = 0u; i < mGraphSize; i++)
    {
        for (unsigned j = 0u; j < mGraphSize; j++)
        {
            in >> table[i][j].weight;
            in.ignore();
        }
    }

}

void NavMesh::SaveToTXT(std::string path)
{
    //OPEN/CREATE FILE
    std::ofstream fileToSave(path);

    //SERIALIZE NUMBER OF NODES
    auto& posArray = mGraph.GetPosArray();

    unsigned nodeNumber = static_cast<unsigned>(posArray.size());
    fileToSave << nodeNumber;
    fileToSave << std::endl;

    //serialize positions
    for (unsigned i = 0u; i < nodeNumber; i++)
    {
        //SAVE NODE POS
        auto NodePos = posArray[i];
        fileToSave << NodePos.x;
        fileToSave << std::endl;
        fileToSave << NodePos.y;
        fileToSave << std::endl;
        fileToSave << NodePos.z;
        fileToSave << std::endl;

        //SAVE ROOM
        /*fileToSave << roomArray[i].max.x << std::endl;
        fileToSave << roomArray[i].max.y << std::endl;
        fileToSave << roomArray[i].max.z << std::endl;

        fileToSave << roomArray[i].min.x << std::endl;
        fileToSave << roomArray[i].min.y << std::endl;
        fileToSave << roomArray[i].min.z << std::endl;*/
    }

    //SERIALIZE MATRIX
    auto& table = mGraph.GetCurrentTable();

    unsigned current = 1u;
    for (auto& it : table) //go through adjacency lists
    {
        //go through the adjacent nodes of this 
        unsigned adjacent = 1u;

        for (auto& it2 : it)
        {
            //serialize wreight
            fileToSave << it2.weight;
            fileToSave << std::endl;
            adjacent++;
        }

        current++;
    }

}
