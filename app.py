from flask import Flask, render_template, request, redirect, url_for, flash, send_file
from flask_sqlalchemy import SQLAlchemy
import csv
import io
import os

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///students.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.secret_key = os.environ.get('FLASK_SECRET', 'change_this_to_a_random_secret')

db = SQLAlchemy(app)

class Student(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(120), nullable=False)
    roll = db.Column(db.String(50), unique=True, nullable=False)
    email = db.Column(db.String(120))
    course = db.Column(db.String(120))
    year = db.Column(db.String(20))
    marks = db.Column(db.Float)

    def __repr__(self):
        return f"<Student {self.name} ({self.roll})>"

@app.before_first_request
def create_tables():
    db.create_all()

@app.route('/')
def index():
    q = request.args.get('q', '').strip()
    if q:
        query = "%" + q + "%"
        students = Student.query.filter(
            (Student.name.ilike(query)) |
            (Student.roll.ilike(query)) |
            (Student.email.ilike(query)) |
            (Student.course.ilike(query))
        ).all()
    else:
        students = Student.query.order_by(Student.id.desc()).all()
    return render_template('index.html', students=students, q=q)

@app.route('/add', methods=['GET', 'POST'])
def add_student():
    if request.method == 'POST':
        name = request.form['name'].strip()
        roll = request.form['roll'].strip()
        email = request.form.get('email','').strip()
        course = request.form.get('course','').strip()
        year = request.form.get('year','').strip()
        marks = request.form.get('marks')
        marks = float(marks) if marks else None

        if not name or not roll:
            flash('Name and Roll number are required.', 'danger')
            return redirect(url_for('add_student'))

        existing = Student.query.filter_by(roll=roll).first()
        if existing:
            flash('A student with that roll number already exists.', 'danger')
            return redirect(url_for('add_student'))

        student = Student(name=name, roll=roll, email=email, course=course, year=year, marks=marks)
        db.session.add(student)
        db.session.commit()
        flash('Student added successfully.', 'success')
        return redirect(url_for('index'))

    return render_template('add_student.html')

@app.route('/edit/<int:student_id>', methods=['GET', 'POST'])
def edit_student(student_id):
    student = Student.query.get_or_404(student_id)
    if request.method == 'POST':
        student.name = request.form['name'].strip()
        student.roll = request.form['roll'].strip()
        student.email = request.form.get('email','').strip()
        student.course = request.form.get('course','').strip()
        student.year = request.form.get('year','').strip()
        marks = request.form.get('marks')
        student.marks = float(marks) if marks else None

        if not student.name or not student.roll:
            flash('Name and Roll number are required.', 'danger')
            return redirect(url_for('edit_student', student_id=student.id))

        # ensure unique roll (allow same as current)
        existing = Student.query.filter(Student.roll==student.roll, Student.id!=student.id).first()
        if existing:
            flash('Another student with that roll number already exists.', 'danger')
            return redirect(url_for('edit_student', student_id=student.id))

        db.session.commit()
        flash('Student updated successfully.', 'success')
        return redirect(url_for('index'))

    return render_template('edit_student.html', student=student)

@app.route('/delete/<int:student_id>', methods=['POST'])
def delete_student(student_id):
    student = Student.query.get_or_404(student_id)
    db.session.delete(student)
    db.session.commit()
    flash('Student deleted.', 'success')
    return redirect(url_for('index'))

@app.route('/export')
def export_csv():
    students = Student.query.order_by(Student.id).all()
    si = io.StringIO()
    cw = csv.writer(si)
    cw.writerow(['id','name','roll','email','course','year','marks'])
    for s in students:
        cw.writerow([s.id, s.name, s.roll, s.email or '', s.course or '', s.year or '', s.marks if s.marks is not None else ''])
    output = io.BytesIO()
    output.write(si.getvalue().encode('utf-8'))
    output.seek(0)
    return send_file(output, mimetype='text/csv', as_attachment=True, download_name='students.csv')

if __name__ == '__main__':
    app.run(debug=True)
