#include "interactivebuttonbase.h"

InteractiveButtonBase::InteractiveButtonBase(QWidget *parent)
    : QPushButton(parent),
      enter_pos(-1, -1), press_pos(-1, -1), mouse_pos(-1, -1), anchor_pos(-1,  -1),
      pressing(false), entering(false),
      water_ripple(false), water_finished(false),
      move_speed(5),
      normal_bg(128, 128, 128, 0), hover_bg(128, 128, 128, 25), press_bg(255, 128, 128, 200),
      hover_speed(10), press_speed(20),
      hover_progress(0), press_progress(0)
{
    setMouseTracking(true); // 鼠标没有按下时也能捕获移动事件

    anchor_timer = new QTimer(this);
    anchor_timer->setInterval(20);
    connect(anchor_timer, SIGNAL(timeout()), this, SLOT(anchorTimeOut()));

    setWaterRipple();
}

void InteractiveButtonBase::setWaterRipple(bool enable)
{
    if (water_ripple == enable) return ;

    water_ripple = enable;
    if (water_ripple)
        press_speed >>= 1; // 水波纹模式需要减慢动画速度
    else
        press_speed <<= 1; // 恢复到原来的速度
}

void InteractiveButtonBase::mousePressEvent(QMouseEvent *event)
{
    mouse_pos = mapFromGlobal(QCursor::pos());

    if (event->button() == Qt::LeftButton)
    {
    	pressing = true;
        press_pos = mouse_pos;
        if (water_ripple)
            water_finished = false;
    }

    return QPushButton::mousePressEvent(event);
}

void InteractiveButtonBase::mouseReleaseEvent(QMouseEvent* event)
{
    if (pressing && event->button() == Qt::LeftButton)
	{
		pressing = false;
	}

    return QPushButton::mouseReleaseEvent(event);
}

void InteractiveButtonBase::mouseMoveEvent(QMouseEvent *event)
{
    mouse_pos = mapFromGlobal(QCursor::pos());

    return QPushButton::mouseMoveEvent(event);
}

void InteractiveButtonBase::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);

    // 绘制背景
    QPainterPath path_back;
    path_back.setFillRule(Qt::WindingFill);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    path_back.addRect(QRect(QPoint(0,0), geometry().size()));

    if (hover_progress)
    {
    	QColor bg_color = hover_bg;
    	bg_color.setAlpha(hover_bg.alpha() * hover_progress / 100);
    	painter.fillPath(path_back, QBrush(bg_color));
    }

    if (press_progress)
    {
        if (!water_ripple || water_finished) // 淡化消失：无水波纹，或者水波纹出现结束了
        {
            QColor bg_color = press_bg;
            bg_color.setAlpha(press_bg.alpha() * press_progress / 100);
            painter.fillPath(path_back, QBrush(bg_color));
        }
        else // 水波纹出现
        {
            int radius = static_cast<int>((geometry().width() > geometry().height() ? geometry().width() : geometry().height()) * 1.42);
            QRect circle(press_pos.x() - radius*press_progress/100,
                        press_pos.y() - radius*press_progress/100,
                        radius*press_progress/50,
                        radius*press_progress/50);
            QPainterPath path;
            path.addEllipse(circle);
            painter.fillPath(path, QBrush(press_bg));
        }
    }


    // 绘制鼠标位置
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.drawEllipse(QRect(anchor_pos.x()-5, anchor_pos.y()-5, 10, 10));

//    return QPushButton::paintEvent(event); // 不绘制父类背景了
}

void InteractiveButtonBase::enterEvent(QEvent *event)
{
    if (!anchor_timer->isActive())
    {
    	anchor_timer->start();
    }
    entering = true;

    return QPushButton::enterEvent(event);
}


void InteractiveButtonBase::leaveEvent(QEvent *event)
{
    entering = false;

    return QPushButton::leaveEvent(event);
}

/**
 * 锚点变成到鼠标位置的定时时钟
 */
void InteractiveButtonBase::anchorTimeOut()
{
    // 背景色
    if (pressing) // 鼠标按下
    {
        if (press_progress < 100)
            press_progress += press_speed;
    }
    else // 鼠标悬浮
    {
        if (press_progress>0) // 如果按下的效果还在
        {
            if (water_ripple) // 水波纹
            {
                if (!water_finished) // 按下的水波纹动画还没有结束
                {
                    press_progress += press_speed;
                    if (press_progress >= 100)
                        water_finished = true;
                }
                else
                {
                    press_progress -= press_speed;
                }
            }
            else
            {
                press_progress -= press_speed;
            }
        }

        if (entering) // 在框内：加深
        {
            if (hover_progress < 100)
                hover_progress += hover_speed;
        }
        else // 在框外：变浅
        {
            if (hover_progress > 0)
                hover_progress -= hover_speed;
        }
    }

    // 锚点
    if (anchor_pos != mouse_pos)
    {
        int delta_x = anchor_pos.x() - mouse_pos.x(),
    		delta_y = anchor_pos.y() - mouse_pos.y();
    	
    	if (delta_x < 0) // 右移
    		anchor_pos.setX( anchor_pos.x() + (move_speed > -delta_x ? -delta_x : move_speed) );
    	else if (delta_x > 0) // 左移
    		anchor_pos.setX( anchor_pos.x() - (move_speed > delta_x ? delta_x : move_speed) );

    	if (delta_y < 0) // 右移
    		anchor_pos.setY( anchor_pos.y() + (move_speed > -delta_y ? -delta_y : move_speed) );
    	else if (delta_y > 0) // 左移
    		anchor_pos.setY( anchor_pos.y() - (move_speed > delta_y ? delta_y : move_speed) );
    }

    update();
}
