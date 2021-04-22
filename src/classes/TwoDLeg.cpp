#include "TwoDLeg.h"

TwoDLeg::TwoDLeg() {}

TwoDLeg::TwoDLeg(vec2 startPos, float floorY, float hipLength, float femurLength, float tibiaLength)
{
    //Hip
    mNodes.push_back(Node(startPos, Color(1, 0, 0), false));
    //Femur
    mNodes.push_back(Node(startPos + vec2(hipLength, 0), Color(0, 1, 0)));
    //Tibia
    mNodes.push_back(Node(startPos + vec2(hipLength + femurLength, 0), Color(0, 0, 1)));
    //Feet
    mNodes.push_back(Node(startPos + vec2(hipLength + femurLength + tibiaLength, 0), Color(1, 1, 0)));

    //Set Femur as Hip's child
    mNodes[0].setChild(&mNodes[1]);

    //Set Hip as Femur's axis
    mNodes[1].setAxis(&mNodes[0]);

    //Set Femur as Tibia's axis
    mNodes[2].setAxis(&mNodes[1]);

    //Set Tibia as Feet's axis
    mNodes[3].setAxis(&mNodes[2]);

    mHipToFemurOffset = mNodes[0].getPos() - mNodes[1].getPos();

    mHipLength = hipLength;
    mHipLengthSqr = mHipLengthSqr * mHipLengthSqr;

    mFemurLength = femurLength;
    mFemurLengthSqr = femurLength * femurLength;

    mTibiaLength = tibiaLength;
    mTibiaLengthSqr = tibiaLength * tibiaLength;

    mLegLength = femurLength + tibiaLength;
    mLegLengthSqr = mLegLength * mLegLength;

    mNodesLastIdx = mNodes.size() - 1;
    Node feetNode = mNodes[mNodesLastIdx];
    moveFeet(vec2(feetNode.getPos().x - tibiaLength, floorY));    
    mSelNodeIdx = -1;
}

void TwoDLeg::mouseDown(vec2 mousePos)
{
    if (isPointInCircleSqr(mousePos, mNodes[mNodesLastIdx].getPos(), Node::sNodeRadiusSqr))
        mSelNodeIdx = mNodesLastIdx;
    else if (isPointInCircleSqr(mousePos, mNodes[0].getPos(), Node::sNodeRadiusSqr))
        mSelNodeIdx = 0;
}

void TwoDLeg::mouseDrag(vec2 mousePos)
{
    mousePos = clamp(mousePos, vec2(0, 0), vec2(app::getWindowWidth(), app::getWindowHeight()));

    if (mSelNodeIdx == 0)
        moveWithIK(mousePos);
    else if (mSelNodeIdx == mNodesLastIdx)
        moveFeet(mousePos);
}

void TwoDLeg::mouseUp(vec2 mousePos)
{
    mFeetLastPos = mNodes[mNodesLastIdx].getPos();
    mSelNodeIdx = -1;
}

void TwoDLeg::moveFeet(vec2 pos)
{
    vec2 femurPos = mNodes[1].getPos();
    vec2 diff = pos - femurPos;

    //Get distance of mousePos to Femur node
    //Since we've calculated the difference, using dot product method with sqrt function
    //will be faster. The distance function might recalculate the difference again)
    float dist = sqrt(dot(diff, diff));

    if (dist > mFemurLength + mTibiaLength)
        dist = mFemurLength + mTibiaLength;
    else if (dist < abs(mFemurLength - mTibiaLength))
        dist = abs(mFemurLength - mTibiaLength);

    //Distance cannot be 0. It will return NaN for femurAngle
    if (dist <= 0)
        dist = 0.000001f;

    float distSqr = dist * dist;
    //Angle of the mousePos to the Femur node
    float mousePosToFemurAngle = atan2(diff.y, diff.x);
    float femurAngle = mousePosToFemurAngle;
    float tibiaAngle = mousePosToFemurAngle;

    //The Femur length, Tibia length and the distance forms a triangle,
    //so use it to calculate the angle of the femur and tibia
    //Ref: Law of Cosines
    //cos(c) = (a^2 + b^2 - c^2) / 2ab

    //Angle between the femur and mousePos to femur node + the angle of the mousePos to the Femur node
    femurAngle = -acos((mFemurLengthSqr + distSqr - mTibiaLengthSqr) / (2 * mFemurLength * dist)) + mousePosToFemurAngle;

    //Angle between the femur and tibia
    tibiaAngle = acos((mFemurLengthSqr + mTibiaLengthSqr - distSqr) / (2 * mFemurLength * mTibiaLength));

    //Get the angle between the 0 degrees and the tibia = 180 degrees - (-femur angle) - tibia angle
    //The (-femur angle) is because it is on the opposite end
    tibiaAngle = M_PI - (-femurAngle) - tibiaAngle;

    mNodes[1].rotate(femurAngle);
    mNodes[2].rotate(tibiaAngle);

    mFeetLastPos = pos;
}

void TwoDLeg::moveWithIK(vec2 pos)
{        
    //Subtract the hipToFemur offset from the mousePos to get the position of the femur node
    vec2 femurNewPos = pos - mHipToFemurOffset;

    //Get the femur to feet distance using the new femur pos
    vec2 feetPos = mNodes[3].getPos();
    vec2 diffFemurToFeet = femurNewPos - feetPos;
    float distFemurToFeetSqr = dot(diffFemurToFeet, diffFemurToFeet);

    //If the distance of femur node new position is longer than the leg length
    if (distFemurToFeetSqr > mLegLengthSqr)
    {
        //Calculate the angle using the new femur node position
        float radians = atan2(diffFemurToFeet.y, diffFemurToFeet.x);
        //Rotate the around the feet node with the leg length and the angle obtained
        //and add the hipToFemur offset
        pos = rotatePoint(feetPos, mLegLength, radians) + mHipToFemurOffset;
    }
    
    mNodes[0].moveTo(pos);

    //Recalculate the IK angles since the position has been moved
    moveFeet(mFeetLastPos);
}

void TwoDLeg::moveTo(vec2 newPos)
{
    mNodes[0].moveTo(newPos);
}

vec2 TwoDLeg::getPos()
{
    return mNodes[0].getPos();
}

vec2 TwoDLeg::getFeetPos()
{
    return mNodes[mNodesLastIdx].getPos();
}

void TwoDLeg::draw()
{
    mNodes[0].draw();
    for (int i = 1; i < mNodes.size(); i++)
    {
        Node node = mNodes[i];
        gl::drawLine(mNodes[i - 1].getPos(), node.getPos());
        node.draw();
    }
}