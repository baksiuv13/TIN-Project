﻿using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SieciowyInkScape
{
    public class DrawingAreaState
    {
        Semaphore semaphore;
        public State state;
        public DrawingObject tempObject;
        public Tools selectedTool;

        public List<DrawingObject> objects;
        public Queue<PendingObject> pendingObjects;
        public int drawingAckTime = 5;

        public enum State
        {
            IDLE, DRAWING, EDITING
        }
        public enum Tools
        {
            NOTHING, MODIFY, LINE, RECTANGLE
        }




        public DrawingAreaState(Point areaSize)
        {
            semaphore = new Semaphore(1, 1);
            this.areaSize = areaSize;
            this.state = State.IDLE;
            selectedTool = Tools.RECTANGLE;
            objects = new List<DrawingObject>();
            pendingObjects = new Queue<PendingObject>();
        }
        public void Access()
        {
            semaphore.WaitOne();
        }
        public void Exit()
        {
            semaphore.Release();
        }


        public void FinalizeObject(MainForm main, DrawingObject obj)
        {
            // TODO - send object
            PendingObject pobj = new PendingObject(DateTime.Now, obj);
            pendingObjects.Enqueue(pobj);
        }
        public void CheckPendingObjects()
        {
            while (pendingObjects.Count > 0 && pendingObjects.Peek().timeStarted + new TimeSpan(0, 0, drawingAckTime) < DateTime.Now)
            {
                pendingObjects.Dequeue();
            }
        }



        public class PendingObject
        {
            public DateTime timeStarted;
            public DrawingObject obj;

            public PendingObject(DateTime timeStarted, DrawingObject obj)
            {
                this.timeStarted = timeStarted;
                this.obj = obj;
            }
        }

        public abstract class DrawingObject
        {
            public enum ObjectType
            {
                CIRCLE, RECTANGLE, LINE
            }

            public ObjectType objectType;

            public Color color;

            public float xpos;
            public float ypos;
        }
        public class LineObject : DrawingObject
        {
            public float xpos2;
            public float ypos2;

            public int thickness;

            public LineObject(float xpos, float ypos, float xpos2, float ypos2, int thickness, Color color)
            {
                this.objectType = ObjectType.LINE;
                this.xpos = xpos;
                this.ypos = ypos;
                this.xpos2 = xpos2;
                this.ypos2 = ypos2;
                this.thickness = thickness;
                this.color = color;
            }
        }
        public class RectangleObject : DrawingObject
        {
            public float xpos2;
            public float ypos2;

            public int thickness;

            public RectangleObject(float xpos, float ypos, float xpos2, float ypos2, int thickness, Color color)
            {
                this.objectType = ObjectType.RECTANGLE;
                this.xpos = xpos;
                this.ypos = ypos;
                this.xpos2 = xpos2;
                this.ypos2 = ypos2;
                this.thickness = thickness;
                this.color = color;
            }
        }

        public class MousePosition
        {
            public float xpos;
            public float ypos;
            public string username;
            public MousePosition(float xpos, float ypos, string username)
            {
                this.xpos = xpos;
                this.ypos = ypos;
                this.username = username;
            }
        }

        public Dictionary<string, MousePosition> mousePositions = new Dictionary<string, MousePosition>();
        public Point areaSize;

    }
}